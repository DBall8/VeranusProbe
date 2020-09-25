#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "veranusDisplay/VeranusDisplay.hpp"
#include "drivers/climateSensor/IClimateSensor.hpp"

// debug
#include "drivers/timer/TicCounter.hpp"
extern ticCounter::TicCounter* pTicCounter;

extern ClimateSensor::IClimateSensor* pClimateSensor;
extern VeranusDisplay* pDisplay;
extern softwareTimer::SoftwareTimer* pUpdateTimer;

void initializeDevices();

#endif