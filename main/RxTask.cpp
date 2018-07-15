#include "RxTask.h"

#include <esp_log.h>

#define CHECK_IN_RANGE(dt,tgt,margin) (dt<(tgt+margin))&&(dt>(tgt-margin))

namespace {
	const char *TAG = "TxTask";

	const int cNumItemsInMessage = 17;
	const int cTicksMargin = 50;
	const int cTicksLow = 240;

	const int cHeaderTicksHigh = 240*4;
	const int cOneTicksHigh = 240*2;
	const int cZeroTicksHigh = 240;

	enum DecodedItem_t {
		INVALID = -1,
		LEVEL_0 = 0,
		LEVEL_1 = 1,
		HEADER = 2
	};

	inline DecodedItem_t decode_item(rmt_item32_t &item) {
		// First make sure that
		//  First part is high, 2nd low
		//  2nd part must be in range to be the low pulse
		//  1st part must be in range to be header, zero or one pulse
		ESP_LOGI(TAG, "DECODE %d:%d - %d:%d", item.level0, item.duration0, item.level1, item.duration1);

		if( item.level0 == 1
				|| item.level1 == 0
				|| item.duration1 > (cTicksLow + cTicksMargin)
				|| item.duration1 < (cTicksLow - cTicksMargin)
				|| item.duration0 < (cZeroTicksHigh - cTicksMargin)
				|| item.duration0 > (cHeaderTicksHigh + cTicksMargin)) {
			ESP_LOGI(TAG, "EARLY INVALID!!!");
			return INVALID;
		}

		// At this point, we know that 2nd part is low, and the right length
		//  and first part is high and possibly one of 0/1/HEADER
		if(item.duration0 <= cZeroTicksHigh + cTicksMargin) {
			return LEVEL_0;
		}

		if( (item.duration0 >= cOneTicksHigh - cTicksMargin)
				&& (item.duration0 <= cOneTicksHigh + cTicksMargin) ) {
			return LEVEL_1;
		}

		if( (item.duration0 >= cHeaderTicksHigh - cTicksMargin)
				&& (item.duration0 <= cHeaderTicksHigh + cTicksMargin) ) {
			return HEADER;
		}

		return INVALID;
	}

	inline rmt_item32_t *decode_message( rmt_item32_t *pCurr, rmt_item32_t *pEnd, uint8_t *data, int *dataSize ) {
		int currentByte=-1;
		int currentBit=0;

		while(pCurr != pEnd) {
			switch(decode_item(*pCurr)) {
				case HEADER:
					ESP_LOGI(TAG, "DECODE::HEADER" );

					if(currentByte<0) {
						currentByte=0;
					} else {
						// Return data
						*dataSize=currentByte;

						// Return current so it can be used in next decode call.
						return pCurr;
					}

					break;
				case LEVEL_1:
					ESP_LOGI(TAG, "DECODE::LEVEL_1" );

					if(currentBit>=8) {
						++currentByte;
						currentBit=0;
					}

					data[currentByte]<<=1;
					++currentBit;

					break;
				case LEVEL_0:
					ESP_LOGI(TAG, "DECODE::LEVEL_0" );

					if(currentBit>=8) {
						++currentByte;
						currentBit=0;
					}

					data[currentByte]<<=0;
					++currentBit;

					break;
				default:
					ESP_LOGI(TAG, "DECODE::UNKNOWN" );

					currentBit=0;
					currentByte=-1;
					break;
			}
			++pCurr;
		}

		*dataSize = currentByte;
		return pCurr;
	}
}

RxTask::RxTask(gpio_num_t pin, rmt_channel_t chan)
	// : Task("Receiver")
	: m_Pin(pin)
	, m_RmtChannel(chan)
	, m_RmtRingBuffer(NULL)
{
	rmt_config_t config;
	config.rmt_mode                  = RMT_MODE_RX;
	config.channel                   = chan;
	config.gpio_num                  = pin;
	config.mem_block_num             = 8-chan;
	config.clk_div                   = 200;
	config.rx_config.filter_en		 = false;
	config.rx_config.filter_ticks_thresh = 180; // If pulse length is shorter than this, ignore it. ( in ticks? )
	config.rx_config.idle_threshold  = 240*5; // If pulse length is longer than this reset to idle ( in ticks? )

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(chan, 1000, 0)); // 1000 == rx buffer size

    //get RMT RX ringbuffer
	ESP_ERROR_CHECK(rmt_get_ringbuf_handle(m_RmtChannel, &m_RmtRingBuffer));
    ESP_ERROR_CHECK(rmt_rx_start(m_RmtChannel, 1));
}

void RxTask::update(void)
{
	size_t rx_size = 0;
	//try to receive data from ringbuffer.
	//RMT driver will push all the data it receives to its ringbuffer.
	//We just need to parse the value and return the spaces of ringbuffer.
	void *buff = xRingbufferReceive(m_RmtRingBuffer, &rx_size, 0);
	if(NULL != buff) {
		ESP_LOGI(TAG, "RECEIVE BUFFER!!!");
		rmt_item32_t* item = (rmt_item32_t*)buff;
		rmt_item32_t *pEnd = item + (rx_size/sizeof(rmt_item32_t));
		uint8_t data[4];
		int dataSize = 4;

		while(item != pEnd) {
			dataSize = 4;
			item = decode_message(item, pEnd, data, &dataSize );
			if(dataSize>0) {
				ESP_LOGI(TAG, "Recieved message size=%d", dataSize );
				// Something was decoded
				// for(int i=0; i<dataSize; ++i) {
				// 	ESP_LOGI(TAG, "Recieved message Byte %d/%d = 0x%x", i, dataSize, data[i] );
				// }
			}
		}
		vRingbufferReturnItem(m_RmtRingBuffer, buff);
	}
}
