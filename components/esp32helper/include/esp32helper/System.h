/*
 * System.h
 *
 *  Created on: May 27, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SYSTEM_H_
#define COMPONENTS_CPP_UTILS_SYSTEM_H_
#include <stdint.h>
#include <string>
#include <esp_system.h>

namespace esp32helper {

/**
 * @brief System wide functions.
 */
class System {
public:
	static void getChipInfo(esp_chip_info_t *info);
	static size_t getFreeHeapSize();
	static std::string getIDFVersion();
	static size_t getMinimumFreeHeapSize();
	static void restart();
};

} // namespace esp32helper

#endif /* COMPONENTS_CPP_UTILS_SYSTEM_H_ */
