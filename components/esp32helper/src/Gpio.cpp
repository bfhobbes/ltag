/*
 * GPIO.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#include "../include/esp32helper/Gpio.h"
#include "../include/esp32helper/GeneralUtils.h"

#include "sdkconfig.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

using namespace esp32helper;

static const char* LOG_TAG = "GPIO";

static bool g_isrServiceInstalled = false;

/**
 * @brief Add an ISR handler to the pin.
 * @param [in] pin The pin to have the ISR associated with it.
 * @param [in] handler The function to be invoked when the interrupt is detected.
 * @param [in] pArgs Optional arguments to pass to the handler.
 */
void Gpio::addISRHandler(
	gpio_num_t pin,
	gpio_isr_t handler,
	void*      pArgs) {

	ESP_LOGD(LOG_TAG, ">> addISRHandler:  pin=%d", pin);

	// If we have not yet installed the ISR service handler, install it now.
	if (g_isrServiceInstalled == false) {
		ESP_LOGD(LOG_TAG, "Installing the global ISR service");
		esp_err_t errRc = ::gpio_install_isr_service(0);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "<< gpio_install_isr_service: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			return;
		}
		g_isrServiceInstalled = true;
	}

	esp_err_t errRc = ::gpio_isr_handler_add(pin, handler, pArgs);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< gpio_isr_handler_add: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	ESP_LOGD(LOG_TAG, "<< addISRHandler");
} // addISRHandler


/**
 * @brief Set the pin high.
 *
 * Ensure that the pin is set to be output prior to calling this method.
 *
 * @param [in] pin The pin to be set high.
 * @return N/A.
 */
void Gpio::high(gpio_num_t pin) {
	write(pin, true);
} // high


/**
 * @brief Determine if the pin is a valid pin for an ESP32 (i.e. is it in range).
 *
 * @param [in] pin The pin number to validate.
 * @return The value of true if the pin is valid and false otherwise.
 */
bool Gpio::inRange(gpio_num_t pin) {
	if (pin>=0 && pin<=39) {
		return true;
	}
	return false;
} // inRange


/**
 * @brief Disable interrupts on the named pin.
 * @param [in] pin The pin to disable interrupts upon.
 * @return N/A.
 */
void Gpio::interruptDisable(gpio_num_t pin) {
	esp_err_t rc = ::gpio_intr_disable(pin);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "interruptDisable: %d", rc);
	}
} // interruptDisable


/**
 * @brief Enable interrupts on the named pin.
 * @param [in] pin The pin to enable interrupts upon.
 * @return N/A.
 */
void Gpio::interruptEnable(gpio_num_t pin) {
	esp_err_t rc = ::gpio_intr_enable(pin);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "interruptEnable: %d", rc);
	}
} // interruptEnable


/**
 * @brief Set the pin low.
 *
 * Ensure that the pin is set to be output prior to calling this method.
 *
 * @param [in] pin The pin to be set low.
 * @return N/A.
 */
void Gpio::low(gpio_num_t pin) {
	write(pin, false);
} // low


/**
 * @brief Read a value from the given pin.
 *
 * Ensure the pin is set as input before calling this method.
 * @param [in] pin The pin to read from.
 * @return True if the pin is high, false if the pin is low.
 */
bool Gpio::read(gpio_num_t pin) {
	return ::gpio_get_level(pin);
} // read


/**
 * @brief Set the pin as input.
 *
 * Set the direction of the pin as input.
 * @param [in] pin The pin to set as input.
 * @return N/A.
 */
void Gpio::setInput(gpio_num_t pin) {
	::gpio_set_direction(pin, GPIO_MODE_INPUT);
} // setInput


/**
 * @brief Set the interrupt type.
 * The type of interrupt can be one of:
 *
 * * GPIO_INTR_ANYEDGE
 * * GPIO_INTR_DISABLE
 * * GPIO_INTR_NEGEDGE
 * * GPIO_INTR_POSEDGE
 * * GPIO_INTR_LOW_LEVEL
 * * GPIO_INTR_HIGH_LEVEL
 *
 * @param [in] pin The pin to set the interrupt upon.
 * @param [in] intrType The type of interrupt.
 * @return N/A.
 */
void Gpio::setInterruptType(
		gpio_num_t pin,
		gpio_int_type_t intrType) {
	esp_err_t rc = ::gpio_set_intr_type(pin, intrType);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "setInterruptType: %d", rc);
	}

} // setInterruptType


/**
 * @brief Set the pin as output.
 *
 * Set the direction of the pin as output.  Note that pins 34 through 39 are input only and can **not** be
 * set as output.
 * @param [in] pin The pin to set as output.
 * @return N/A.
 */
void Gpio::setOutput(gpio_num_t pin) {
	::gpio_set_direction(pin, GPIO_MODE_OUTPUT);
} // setOutput


/**
 * @brief Write a value to the given pin.
 *
 * Ensure that the pin is set as output before calling this method.
 * @param [in] pin The gpio pin to change.
 * @param [out] value The value to be written to the pin.
 * @return N/A.
 */
void Gpio::write(gpio_num_t pin, bool value) {
	//ESP_LOGD(LOG_TAG, ">> write: pin: %d, value: %d", pin, value);
	esp_err_t errRc = ::gpio_set_level(pin, value);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< gpio_set_level: pin=%d, rc=%d %s", pin, errRc, GeneralUtils::errorToString(errRc));
	}
} // write


/**
 * @brief Write up to 8 bits of data to a set of pins.
 * @param [in] pins An array of pins to set their values.
 * @param [in] value The data value to write.
 * @param [in] bits The number of bits to write.
 */
void Gpio::writeByte(gpio_num_t pins[], uint8_t value, int bits) {
	ESP_LOGD(LOG_TAG, ">> writeByte: value: %.2x, bits: %d", value, bits);
	for (int i=0; i<bits; i++) {
		//ESP_LOGD(LOG_TAG, "i=%d, bits=%d", i, bits);
		write(pins[i], (value & (1<<i)) != 0);
	}
	ESP_LOGD(LOG_TAG, "<< writeByte");
} // writeByte

/**
 * @brief Set enable GPIO pin's internal pull up
 * @param [in] pin Gpio pin to change
 */
void Gpio::setPullUp(gpio_num_t pin) {
	::gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
}

/**
 * @brief Set enable GPIO pin's internal pull down
 * @param [in] pin Gpio pin to change
 */
void Gpio::setPullDown(gpio_num_t pin) {
	::gpio_set_pull_mode(pin, GPIO_PULLDOWN_ONLY);
}

/**
 * @brief Set GPIO pin to floatin
 * @param [in] pin Gpio pin to change
 */
void Gpio::setFloating(gpio_num_t pin) {
	::gpio_set_pull_mode(pin, GPIO_FLOATING);
}
