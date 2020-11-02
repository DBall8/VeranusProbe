#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "version.hpp"
#include "config.hpp"
#include "veranusReceiver/ReceiverCli.hpp"

void loop();

int main(void)
{
  initializeDevices();

  pCli->enable();

#ifdef DEBUG
  PRINTLN("Running build %d.%d", V_MAJOR, V_MINOR);
  PRINT("Probes:");
  for (uint8_t i=0; i<NUM_PROBES; i++)
  {
    PRINT(" %d,", probeIds[i]);
  }
  PRINTLN("");
#endif

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
  pCli->update();
}