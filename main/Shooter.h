#pragma once
#include "esp32helper/Task.h"
#include "esp32helper/Rmt.h"

class ShooterTask : public esp32helper::Task {
public:
	ShooterTask();
	virtual void run(void *data);

private:
	esp32helper::Rmt *m_Rmt;;

};