#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/radio/IRadio.hpp"
#include "drivers/uart/IUart.hpp"

extern Uart::IUart* pUart;
extern Radio::IRadio* pRadio;
extern Tic::TicCounter* pTicHandler;
extern Timer::SoftwareTimer* pTimeoutTimer;
extern Timer::SoftwareTimer* pRequestTimer;
extern Timer::SoftwareTimer* pUpdateTimer;

void initializeDevices();

#endif