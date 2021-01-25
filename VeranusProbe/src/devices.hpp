#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "veranusDisplay/VeranusDisplay.hpp"
#include "drivers/climateSensor/IClimateSensor.hpp"
#include "drivers/phototransistor/PhotoTransistor.hpp"

extern ClimateSensor::IClimateSensor* pClimateSensor;
extern PhotoTransistor* pLightSensor;
extern VeranusDisplay* pDisplay;
extern Timer::SoftwareTimer* pUpdateTimer;
extern Timer::SoftwareTimer* pClimateTimer;
extern Timer::SoftwareTimer* pLightTimer;

// Todo replace with wifi driver
#include "drivers/serial/ISerial.hpp"
extern Serial::ISerial* pWifiSerial;

void initializeDevices();

#endif