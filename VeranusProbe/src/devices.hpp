#ifndef DEVICES_HPP
#define DEVICES_HPP

#include "drivers/timer/SoftwareTimer.hpp"
#include "veranusDisplay/VeranusDisplay.hpp"
#include "veranusProbe/VeranusProbe.hpp"
#include "drivers/serial/ISerial.hpp"
#include "wifiInterface/WifiInterface.hpp"
#include "drivers/watchdog/Watchdog.hpp"
#include "drivers/eeprom/EepromManager.hpp"

extern VeranusProbe* pProbe;
extern VeranusDisplay* pDisplay;
extern Timer::SoftwareTimer* pUpdateTimer;
extern Timer::SoftwareTimer* pClimateTimer;
extern Timer::SoftwareTimer* pLightTimer;
extern SerialComm::ISerial* pSerial;
extern WifiInterface* pWifiInterface;
extern Watchdog::IWatchdog* pWdt;
extern Eeprom::EepromManager* pEepromManager;
void initializeDevices();

struct VeranusData
{
  float tempF;
  float humidity;
  float light;
};
const static uint8_t V_DATA_SIZE = sizeof(VeranusData);

extern VeranusData latestData;

#endif