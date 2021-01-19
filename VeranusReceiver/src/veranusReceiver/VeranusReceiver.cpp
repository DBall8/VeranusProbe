#include "VeranusReceiver.hpp"

#include "utilities/print/Print.hpp"
#include "utilities/Conversions.hpp"
#include "drivers/timer/Delay.hpp"

static uint32_t successes = 0;
static uint32_t failures = 0;

const static uint8_t FAILURE_CODE = 0xff;

VeranusReceiver::VeranusReceiver(Radio::IRadio* pRadio,
                                 Uart::IUart* pUart,
                                 Timer::SoftwareTimer* pTimeoutTimer):
    pRadio_(pRadio),
    pUart_(pUart),
    pTimeoutTimer_(pTimeoutTimer)
{

}

VeranusReceiver::~VeranusReceiver(){}

bool VeranusReceiver::request(uint8_t probeId)
{
  pRadio_->setPayloadSize(ID_SIZE);
  if (!pRadio_->startTransmitting(probeId))
  {
#ifdef DEBUG
    PRINTLN("Failed to start transmiting.");
#endif
    return false;
  }

  //DELAY(1);

  return pRadio_->transmit(&probeId, ID_SIZE);
}

bool VeranusReceiver::receive(uint8_t probeId)
{
  pRadio_->setPayloadSize(V_DATA_SIZE);
  if (!pRadio_->startReceiving(probeId))
  {
#ifdef DEBUG
    PRINTLN("Failed to start receiving.");
#endif
    return false;
  }

  pTimeoutTimer_->enable();
  while (!pRadio_->isDataAvailable())
  {
    // Check for a timeout so we do not wait forever
    if (pTimeoutTimer_->hasOneShotPassed())
    {
      pTimeoutTimer_->disable();
      return false;
    }
  }

  VeranusData incomingData;
  bool success = pRadio_->receive((uint8_t*)&incomingData, V_DATA_SIZE);

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

void VeranusReceiver::getUpdate(uint8_t probeId)
{
    // Request an update from each probe
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
        successes++;
    }
    else
    {
        transmitFailResponse();
        failures++;
    }

#ifdef DEBUG
    PRINTLN("Successes: %d, failures: %d", (uint16_t)successes, (uint16_t)failures);
#endif
}

void VeranusReceiver::transmitOverUart(VeranusData data)
{
  VeranusTransmission transmission;
  transmission.probeId = data.probeId;
  transmission.light = data.light;
  transmission.temp = data.tempF;
  transmission.humid = data.humidity;
  pUart_->write((uint8_t*)&transmission, sizeof(transmission));
}

void VeranusReceiver::transmitFailResponse()
{
    pUart_->write((uint8_t*)&FAILURE_CODE, sizeof(FAILURE_CODE));
}