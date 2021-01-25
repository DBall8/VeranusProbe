#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "version.hpp"
#include "config.hpp"

const static uint8_t PROBE_ID = 3;

const static uint8_t ID_SIZE = sizeof(PROBE_ID);

struct VeranusData
{
  uint8_t probeId;
  float tempF;
  float humidity;
  float light;
};
const static uint8_t V_DATA_SIZE = sizeof(VeranusData);

static VeranusData latestData = 
{
  .probeId = PROBE_ID,
  .tempF = 0,
  .humidity = 0,
  .light = 0
};

void updateReceiver();
void updateLightSensor();
void updateClimateSensor();
void loop();

int main(void)
{
  initializeDevices();

  PRINTLN("Running build %d.%d", V_MAJOR, V_MINOR);
  PRINTLN("Probe ID: %d", PROBE_ID);

  // Get initial readings for each sensor
  updateClimateSensor();
  updateLightSensor();

  // Start the update timers
  pUpdateTimer->enable();
  pClimateTimer->enable();
  pLightTimer->enable();

  for (;;) {

    if (pUpdateTimer->hasPeriodPassed())
    {
		  loop();
    }
	}

  return 0;
}

void loop()
{
  const static uint8_t rxBuffLen = 10;
  static uint8_t rxBuff[rxBuffLen];

  if (pWifiSerial->isDataAvailable())
  {
    uint16_t numBytes = pWifiSerial->read(rxBuff, rxBuffLen);
    
    for (uint8_t i=0; i<numBytes; i++)
    {
      rxBuff[i]++;
    }

    pWifiSerial->write(rxBuff, numBytes);
  }

  // static uint8_t d = (uint8_t)'b';
  // pWifiSerial->write(&d, 1);
  // d++;

  // if (pClimateTimer->hasPeriodPassed())
  // {
  //   updateClimateSensor();

  //   // Reset timer to ensure we do not try and read the sensor
  //   // again before it is ready
  //   pClimateTimer->reset();
  // }

  // if (pLightTimer->hasPeriodPassed())
  // {
  //   updateLightSensor();
  // }

  // TODO - notify wifi of update, at desired interval
}

void updateClimateSensor()
{
  // Try and get a new climate reading
  if (pClimateSensor->update())
  {
    latestData.tempF = pClimateSensor->getTemperatureFahrenheit();
    latestData.humidity = pClimateSensor->getRelativeHumidity();

    DEBUG_PRINTLN("Temp: %fF, Humidity: %f%", latestData.tempF, latestData.humidity);

    // Update display
    pDisplay->update(latestData.tempF, latestData.humidity);
  }
  else
  {
    DEBUG_PRINTLN("Failed to read from climate sensor.");
  }
}

void updateLightSensor()
{
  pLightSensor->update();
  latestData.light = pLightSensor->getLightPercent();

  DEBUG_PRINTLN("Light: %d%", (uint16_t)latestData.light);
}