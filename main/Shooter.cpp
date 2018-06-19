#include "Shooter.h"

// #include "esp32helper/U8g2.h"
// #include "esp32helper/Gpio.h"
// #include "esp32helper/FreeRtos.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"

// #include "soc/soc.h"

#include <esp_log.h>

#include "sdkconfig.h"

static const char *TAG = "ltag";

using namespace esp32helper;

#if defined(CONFIG_LTAG_TX_ENABLE)
#	define RMT_TX_PIN (gpio_num_t)CONFIG_LTAG_TX_PIN
#endif

ShooterTask::ShooterTask(gpio_num_t pin, rmt_channel_t chan)
: Task("Shooter")
, m_Pin(pin)
, m_RmtChannel(chan)
{
	m_ShotQueue = xQueueCreate(10, sizeof(gpio_num_t));

	rmt_config_t config;
	config.rmt_mode                  = RMT_MODE_TX;
	config.channel                   = chan;
	config.gpio_num                  = pin;
	config.mem_block_num             = 8-chan;
	config.clk_div                   = 200;
	config.tx_config.loop_en         = 0;
	config.tx_config.carrier_en      = 1;
	config.tx_config.idle_output_en  = 1;
	config.tx_config.idle_level      = (rmt_idle_level_t)0;
	config.tx_config.carrier_freq_hz = 56000;
	config.tx_config.carrier_level   = (rmt_carrier_level_t)1;
	config.tx_config.carrier_duty_percent = 50;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(chan, 0, 0));
}

void ShooterTask::run(void *data) {
	QueueHandle_t q1 = static_cast<QueueHandle_t>(data);

	while(1) {
		ESP_LOGI(TAG, "Waiting on Trigger queue");
		gpio_num_t pin;
		BaseType_t rc = xQueueReceive(q1, &pin, portMAX_DELAY);
		ESP_LOGI(TAG, "Shooter Woke from interrupt queue wait: %d - %d", rc, pin);

		// TX header and 14 bits which is 15 hi/lo rmt_item32_t's
		uint8_t playerId(0x1);
		uint8_t damage(0x0 << 4 | 100); // Test data. Team 0, 100 points damage

		rmt_item32_t items[15];
		int idx = 0;
		items[idx].level0 = 1;
		items[idx].duration0 = 240*4; // 2400uS
		items[idx].level1 = 0;
		items[idx].duration1 = 240; // 600uS

		++idx;

		// MilesTag spec is 14 bits ID(8 bits) + Damage(6 bits)
		uint8_t byte = playerId & 0x7F;
		while(idx < 9) {
			items[idx].level0 = 1;
			if(byte&0x80) {
				items[idx].duration0 = 240*2; // 1200uS
			} else {
				items[idx].duration0 = 240; // 600uS
			}
			items[idx].level1 = 0;
			items[idx].duration1 = 240; // 600uS			}

			byte<<=1;
			++idx;
		}

		// data we need is bottom 6 bits
		byte = damage << 2;
		while(idx < 15) {
			items[idx].level0 = 1;
			if(byte&0x80) {
				items[idx].duration0 = 240*2; // 1200uS
			} else {
				items[idx].duration0 = 240; // 600uS
			}
			items[idx].level1 = 0;
			items[idx].duration1 = 240; // 600uS			}

			byte<<=1;
			++idx;
		}

		ESP_ERROR_CHECK(::rmt_write_items(m_RmtChannel, &items[0], idx, true));
	}
}

void ShooterTask::queueShot() {
	gpio_num_t pin((gpio_num_t)20);
	xQueueSendToBackFromISR(m_ShotQueue, &pin, NULL);
}