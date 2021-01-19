#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/radio/IRadio.hpp"
#include "drivers/uart/IUart.hpp"
#include "veranusReceiver/VeranusReceiver.hpp"

#include <stdint.h>

extern Uart::IUart* pUart;
extern Timer::SoftwareTimer* pUpdateTimer;
extern VeranusReceiver* pVeranusReceiver;

void initializeDevices();

#endif