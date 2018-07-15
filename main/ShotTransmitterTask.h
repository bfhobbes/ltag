#pragma once
#include "esp32helper/Task.h"

#include <driver/rmt.h>


class ShotTransmitterTask : public esp32helper::Task {
public:
	ShotTransmitterTask(gpio_num_t pin, rmt_channel_t chan);

	void queueShot();

protected:
	virtual void run(void *data);

private:
	gpio_num_t m_Pin;
	rmt_channel_t m_RmtChannel;
	QueueHandle_t m_ShotQueue;
};