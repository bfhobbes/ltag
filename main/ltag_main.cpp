 /* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "Shooter.h"
#include "RxTask.h"

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

// const gpio_num_t RST_PIN = (gpio_num_t)16;
// const gpio_num_t SDA_PIN = (gpio_num_t)4;
// const gpio_num_t SCL_PIN = (gpio_num_t)15;
// const gpio_num_t TRIGGER_PIN = (gpio_num_t)21;

static const char *TAG = "ltag";

extern "C" {
	void app_main(void);
}

const gpio_num_t cTriggerPin = (gpio_num_t)CONFIG_LTAG_TRIGGER_PIN;
const uint32_t cDebounceTime = 1000;

static ShooterTask *g_Shooter = NULL;
static RxTask *g_Receiver;

static uint32_t g_LastTime = 0;
void IRAM_ATTR isrRoutine(void *val) {
	uint32_t timeNow = FreeRtos::getTimeSinceStart();
	if(timeNow-g_LastTime > cDebounceTime) {
		g_Shooter->queueShot();

		g_LastTime = timeNow;
	}
}

void app_main()
{
	g_Shooter = new ShooterTask((gpio_num_t)CONFIG_LTAG_TX_PIN, RMT_CHANNEL_0);
	g_Receiver = new RxTask((gpio_num_t)CONFIG_LTAG_RX_PIN, RMT_CHANNEL_1);

	g_Shooter->start();
	g_Receiver->start();

	// Setup Trigger pin and wire it up to isr routine to queue a shot
	Gpio::setInput(cTriggerPin);
	Gpio::setInterruptType(cTriggerPin, GPIO_INTR_POSEDGE);
	Gpio::setPullDown(cTriggerPin);
	Gpio::addISRHandler(cTriggerPin, isrRoutine, NULL);
	Gpio::interruptEnable(cTriggerPin);
	//Gpio::interruptDisable(TRIGGER_PIN);

	// gpio_isr_handle_t

	// Reset the OLED display
	// gpio_pad_select_gpio(RST_PIN);
	// ESP_ERROR_CHECK(gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT));
	// ESP_ERROR_CHECK(gpio_set_level(RST_PIN, 1));

	// OLED display : TODO
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
