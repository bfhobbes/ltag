#include "RxTask.h"

RxTask::RxTask(gpio_num_t pin, rmt_channel_t chan)
: Task("Shooter")
, m_Pin(pin)
, m_RmtChannel(chan)
{
}
