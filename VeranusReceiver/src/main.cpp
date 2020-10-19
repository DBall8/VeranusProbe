#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "version.hpp"
#include "config.hpp"
#include "utilities/Conversions.hpp"

const static uint16_t REQUEST_TIME_SEC_DEFAULT = 60 * 15; // Default time between requests for data
const static uint16_t REQUEST_TIME_SEC_FAILURE = 60 * 5; // Time between requests after a failure

const static uint8_t ID_SIZE = sizeof(uint8_t);

const static uint8_t probeIds[] =
{
  1
};
const static uint8_t NUM_PROBES = sizeof(probeIds);

struct VeranusData
{
  uint8_t probeId;
  float tempF;
  float humidity;
  float light;
};
const static uint8_t V_DATA_SIZE = sizeof(VeranusData);

struct VeranusTransmission
{
  uint8_t startCode = 0x7f;
  float light = 0;
  float temp = 0;
  float humid = 0;
  uint8_t endCode = 0xff;
};

static uint32_t successes = 0;
static uint32_t failures = 0;

void transmitOverUart(VeranusData data);
void getUpdates();
void loop();

int main(void)
{
  initializeDevices();

  pRadio->enable();

#ifdef DEBUG
  PRINTLN("Running build %d.%d", V_MAJOR, V_MINOR);
  PRINT("Probes:");
  for (uint8_t i=0; i<NUM_PROBES; i++)
  {
    PRINT(" %d,", probeIds[i]);
  }
  PRINTLN("");
#endif

  getUpdates();

  for (;;) {

    if (pUpdateTimer->hasPeriodPassed())
    {
		  loop();
    }
	}

  return 0;
}

bool request(uint8_t probeId)
{
  pRadio->setPayloadSize(ID_SIZE);
  if (!pRadio->startTransmitting(probeId))
  {
#ifdef DEBUG
    PRINTLN("Failed to start transmiting.");
#endif
    return false;
  }

  return pRadio->transmit(&probeId, ID_SIZE);
}

bool receive(uint8_t probeId)
{
  pRadio->setPayloadSize(V_DATA_SIZE);
  if (!pRadio->startReceiving(probeId))
  {
#ifdef DEBUG
    PRINTLN("Failed to start receiving.");
#endif
    return false;
  }

  pTimeoutTimer->enable();
  while (!pRadio->isDataAvailable())
  {
    // Check for a timeout so we do not wait forever
    if (pTimeoutTimer->hasOneShotPassed())
    {
      pTimeoutTimer->disable();
      return false;
    }
  }

  VeranusData incomingData;
  bool success = pRadio->receive((uint8_t*)&incomingData, V_DATA_SIZE);

  if (success)
  {
    transmitOverUart(incomingData);
  }

#ifdef DEBUG
  PRINTLN("Received from %d: T: %dF, H: %d%, L: %d%",
    incomingData.probeId,
    (int16_t)incomingData.tempF,
    (int16_t)incomingData.humidity,
    (int16_t)incomingData.light);
#endif
  
  return success;
}

void getUpdates()
{
  // Request an update from each probe
  for (uint8_t i=0; i<NUM_PROBES; i++)
  {
    uint8_t probeId = probeIds[i];
    bool success = request(probeId);

#ifdef DEBUG
    PRINTLN("Request for %d: %s", probeId, (success ? "SUCCESS" : "FAIL"));
#endif

    if (success)
    {
      success = receive(probeId);

#ifdef DEBUG
      PRINTLN("Receive from %d: %s", probeId, (success ? "SUCCESS" : "FAIL"));
#endif
    }

    // Track successful vs failed transactions
    if (success)
    {
      pRequestTimer->setPeriod(pTicHandler->secondsToTics(REQUEST_TIME_SEC_DEFAULT));
      successes++;
    }
    else
    {
      pRequestTimer->setPeriod(pTicHandler->secondsToTics(REQUEST_TIME_SEC_FAILURE));
      failures++;
    }

#ifdef DEBUG
    PRINTLN("Successes: %d, failures: %d", (uint16_t)successes, (uint16_t)failures);
#endif
  }
}

void transmitOverUart(VeranusData data)
{
  VeranusTransmission transmission;
  transmission.light = data.light;
  transmission.temp = degreesFToC(data.tempF);
  transmission.humid = data.humidity;
  pUart->write((uint8_t*)&transmission, sizeof(transmission));
}

void loop()
{
  if (pRequestTimer->hasPeriodPassed())
  {
    getUpdates();
    pRequestTimer->reset();
  }
}