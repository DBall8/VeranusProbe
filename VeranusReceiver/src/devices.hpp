#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/radio/IRadio.hpp"
#include "drivers/uart/IUart.hpp"

extern uart::IUart* pUart;
extern radio::IRadio* pRadio;
extern ticCounter::TicCounter* pTicHandler;
extern softwareTimer::SoftwareTimer* pTimeoutTimer;
extern softwareTimer::SoftwareTimer* pRequestTimer;
extern softwareTimer::SoftwareTimer* pUpdateTimer;

void initializeDevices();

#endif