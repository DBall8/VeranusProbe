#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "version.hpp"

void updateSensors();
void loop();

int main(void)
{
  initializeDevices();

  PRINTLN("Running build %d.%d", V_MAJOR, V_MINOR);

  updateSensors();

  for (;;) {

    if (pUpdateTimer->hasPeriodPassed())
    {
		  loop();
    }
	}

  return 0;
}

static uint32_t lastTic = 0;
void loop()
{
  updateSensors();
}

void updateSensors()
{
  int16_t tempF = 0;
  int16_t humidity = 0;

  if (pClimateSensor->update())
  {
    tempF = pClimateSensor->getTemperatureFahrenheit();
    humidity = pClimateSensor->getRelativeHumidity();
    PRINTLN("Temp: %dF, Humidity: %d%", tempF, humidity);
  }
  else
  {
    PRINTLN("Failed to read from climate sensor.");
  }

  pDisplay->update(tempF, humidity);
}