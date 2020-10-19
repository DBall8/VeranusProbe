#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "veranusDisplay/VeranusDisplay.hpp"
#include "drivers/climateSensor/IClimateSensor.hpp"
#include "drivers/phototransistor/PhotoTransistor.hpp"
#include "drivers/radio/IRadio.hpp"

extern ClimateSensor::IClimateSensor* pClimateSensor;
extern PhotoTransistor* pLightSensor;
extern VeranusDisplay* pDisplay;
extern Radio::IRadio* pRadio;
extern Timer::SoftwareTimer* pUpdateTimer;
extern Timer::SoftwareTimer* pClimateTimer;
extern Timer::SoftwareTimer* pLightTimer;

void initializeDevices();

#endif