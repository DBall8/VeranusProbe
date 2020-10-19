#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "version.hpp"

const static uint8_t PROBE_ID = 1;

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

  // Initialize radio to listen for requests for data from the Receiver
  pRadio->enable();
  pRadio->setPayloadSize(ID_SIZE);
  pRadio->startReceiving(PROBE_ID);

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
  if (pClimateTimer->hasPeriodPassed())
  {
    updateClimateSensor();

    // Reset timer to ensure we do not try and read the sensor
    // again before it is ready
    pClimateTimer->reset();
  }

  if (pLightTimer->hasPeriodPassed())
  {
    updateLightSensor();
  }

  // Check if an update has been requested by the receiver
  updateReceiver();
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

void updateReceiver()
{
  // Check if the receiver has requested any data from us
  if (pRadio->isDataAvailable())
  {
    // Receive the ID the receiver is requesting to hear from
    uint8_t id = 0;
    pRadio->receive(&id, ID_SIZE);

    if (id != PROBE_ID)
    {
      // This is the wrong ID for this probe, this should never happen
      DEBUG_PRINTLN("Received wrong id: %d", id);
      return;
    }

    // Receiver wants our latest data, so send it
    pRadio->setPayloadSize(V_DATA_SIZE);
    pRadio->startTransmitting(PROBE_ID);
    if (!pRadio->transmit((uint8_t*)&latestData, V_DATA_SIZE))
    {
      DEBUG_PRINTLN("Transmission failed.");
    }

    pRadio->setPayloadSize(ID_SIZE);
    pRadio->startReceiving(PROBE_ID);
  }
}