 /* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "Shooter.h"

#include "esp32helper/Rmt.h"
#include "esp32helper/U8g2.h"
#include "esp32helper/Gpio.h"
#include "esp32helper/FreeRtos.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "soc/soc.h"

// #include "esp_system.h"
// #include "esp_spi_flash.h"

// #include "u8x8.h"
// #include "u8g2_esp32_hal.h"

#include <esp_log.h>

#include <string.h>

#include "sdkconfig.h"

using namespace esp32helper;

#define TX_EN CONFIG_LTAG_TX_ENABLE
#define RX_EN CONFIG_LTAG_RX_ENABLE

const gpio_num_t RST_PIN = (gpio_num_t)16;
const gpio_num_t SDA_PIN = (gpio_num_t)4;
const gpio_num_t SCL_PIN = (gpio_num_t)15;
const gpio_num_t TRIGGER_PIN = (gpio_num_t)21;

#define RMT_TX_CARRIER_EN    1   /*!< Enable carrier for IR transmitter test with IR led */

// GPIO Pins
#define I2C0_SDA_PIN 4
#define I2C0_SCL_PIN 15
#define OLED_RST_PIN 16

#if defined(CONFIG_LTAG_RX_ENABLE)
#	define RMT_RX_PIN CONFIG_LTAG_RX_PIN
#endif

#if defined(CONFIG_LTAG_TX_ENABLE)
#	define RMT_TX_PIN (gpio_num_t)CONFIG_LTAG_TX_PIN
#endif

// Channels
#define RMT_TX_CHANNEL 0
#define RMT_RX_CHANNEL 1

#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

// Andy - what is this?
#define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */

#define RMT_RX_ACTIVE_LEVEL  0   /*!< If we connect with a IR receiver, the data is active low */

#define NEC_HEADER_HIGH_US    9000                         /*!< NEC protocol header: positive 9ms */
#define NEC_HEADER_LOW_US     4500                         /*!< NEC protocol header: negative 4.5ms*/
#define NEC_BIT_ONE_HIGH_US    560                         /*!< NEC protocol data bit 1: positive 0.56ms */
#define NEC_BIT_ONE_LOW_US    (2250-NEC_BIT_ONE_HIGH_US)   /*!< NEC protocol data bit 1: negative 1.69ms */
#define NEC_BIT_ZERO_HIGH_US   560                         /*!< NEC protocol data bit 0: positive 0.56ms */
#define NEC_BIT_ZERO_LOW_US   (1120-NEC_BIT_ZERO_HIGH_US)  /*!< NEC protocol data bit 0: negative 0.56ms */
#define NEC_BIT_END            560                         /*!< NEC protocol end: positive 0.56ms */
// #define NEC_BIT_MARGIN         20                          /*!< NEC parse margin time */
#define NEC_BIT_MARGIN         100                          /*!< NEC parse margin time */

#define NEC_ITEM_DURATION(d)  ((d & 0x7fff)*10/RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
#define NEC_DATA_ITEM_NUM   34  /*!< NEC code item number: header + 32bit data + end */
#define RMT_TX_DATA_NUM  100    /*!< NEC tx test data number */


static const char *TAG = "ltag";
static const char *NEC_TAG = "nec_tag";

static QueueHandle_t q1;

extern "C" {
	void app_main(void);
}

const uint32_t cDebounceTime = 1000;
static uint32_t g_LastTime = 0;

void IRAM_ATTR isrRoutine(void *val) {
	uint32_t timeNow = FreeRtos::getTimeSinceStart();
	if(timeNow-g_LastTime > cDebounceTime) {
		gpio_num_t pin(TRIGGER_PIN);
		xQueueSendToBackFromISR(q1, &pin, NULL);
		g_LastTime = timeNow;
	}
}

void app_main()
{
	q1 = xQueueCreate(10, sizeof(gpio_num_t));
	Gpio::setInput(TRIGGER_PIN);
	Gpio::setInterruptType(TRIGGER_PIN, GPIO_INTR_POSEDGE);
	Gpio::setPullDown(TRIGGER_PIN);
	Gpio::addISRHandler(TRIGGER_PIN, isrRoutine, NULL);
	Gpio::interruptEnable(TRIGGER_PIN);
	//Gpio::interruptDisable(TRIGGER_PIN);

	// gpio_isr_handle_t

	gpio_pad_select_gpio(RST_PIN);
	ESP_ERROR_CHECK(gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(RST_PIN, 1));

    ESP_LOGI(TAG, "TICKS PER 10uSec %d", RMT_TICK_10_US);
    ESP_LOGI(TAG, "APB_CLK_FREQ %d", APB_CLK_FREQ);

	ShooterTask *shooter = new ShooterTask();
	shooter->start(q1);

	// U8g2 disp(SDA_PIN, SCL_PIN, 0x3c);
	// disp.initDisplay();
	// disp.setPowerSave(0);
	// disp.setFont(u8g2_font_unifont_t_symbols);

	// disp.clearBuffer();
	// disp.drawFrame(0,26,100,6);
	// disp.drawUTF8(0,20,"...Monkey...");
	// disp.sendBuffer();

	while(1) {
		Task::delay(1000);
	}
	vTaskDelete(NULL);


// #if RX_EN
//     xTaskCreate(ltag_ir_recv_task, "rmt_nec_rx_task", 2048, NULL, 10, NULL);
// #endif

// #if TX_EN
//     xTaskCreate(rmt_example_nec_tx_task, "rmt_nec_tx_task", 2048, NULL, 10, NULL);
// #endif
}
