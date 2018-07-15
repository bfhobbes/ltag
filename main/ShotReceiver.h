#pragma once

#include "esp32helper/Task.h"

#include <driver/rmt.h>

class ShotReceiver // : public esp32helper::Task
{
public:
	ShotReceiver(gpio_num_t pin, rmt_channel_t chan);

	void update();
protected:
	// virtual void run(void *data);


private:
	gpio_num_t m_Pin;
	rmt_channel_t m_RmtChannel;
	RingbufHandle_t m_RmtRingBuffer;
};