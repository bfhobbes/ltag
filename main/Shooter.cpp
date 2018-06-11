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

ShooterTask::ShooterTask()
: Task("Shooter")
{
	m_Rmt = new Rmt(RMT_TX_PIN);
}

void ShooterTask::run(void *data) {
	QueueHandle_t q1 = static_cast<QueueHandle_t>(data);

	while(1) {
		ESP_LOGI(TAG, "Waiting on Trigger queue");
		gpio_num_t pin;
		BaseType_t rc = xQueueReceive(q1, &pin, portMAX_DELAY);
		ESP_LOGI(TAG, "Shooter Woke from interrupt queue wait: %d - %d", rc, pin);
		m_Rmt->add(true, 32767);
		m_Rmt->add(false, 32767);
		m_Rmt->add(true, 32767);
		m_Rmt->add(false, 15000);
		m_Rmt->add(true, 15000);
		m_Rmt->add(false, 32767);
		m_Rmt->write();

	}
}