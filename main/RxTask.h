#pragma once

#include "esp32helper/Task.h"

#include <driver/rmt.h>

class RxTask : public esp32helper::Task {
public:
	RxTask(gpio_num_t pin, rmt_channel_t chan);

private:
	gpio_num_t m_Pin;
	rmt_channel_t m_RmtChannel;
};