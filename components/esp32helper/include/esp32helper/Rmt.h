/*
 * RMT.h
 *
 *  Created on: Mar 1, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_RMT_H_
#define COMPONENTS_CPP_UTILS_RMT_H_
#include <driver/rmt.h>
#include <vector>

namespace esp32helper {
/**
 * @brief Drive the %RMT peripheral.
 */
class Rmt {
public:
	Rmt(gpio_num_t pin, rmt_channel_t channel=RMT_CHANNEL_0);
	virtual ~Rmt();
	void add(bool level, uint32_t duration);
	void clear();
	void rxStart();
	void rxStop();
	void txStart();
	void txStop();
	void write();


private:
	rmt_channel_t channel;
	std::vector<rmt_item32_t> items;
	int bitCount = 0;
};

} // namespace esp32helper

#endif /* COMPONENTS_CPP_UTILS_RMT_H_ */
