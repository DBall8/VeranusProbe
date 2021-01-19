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