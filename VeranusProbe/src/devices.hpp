#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "veranusDisplay/VeranusDisplay.hpp"
#include "drivers/climateSensor/IClimateSensor.hpp"
#include "drivers/phototransistor/PhotoTransistor.hpp"
#include "drivers/radio/IRadio.hpp"

extern ClimateSensor::IClimateSensor* pClimateSensor;
extern photoTransistor::PhotoTransistor* pLightSensor;
extern VeranusDisplay* pDisplay;
extern radio::IRadio* pRadio;
extern softwareTimer::SoftwareTimer* pUpdateTimer;
extern softwareTimer::SoftwareTimer* pClimateTimer;
extern softwareTimer::SoftwareTimer* pLightTimer;


void initializeDevices();

#endif