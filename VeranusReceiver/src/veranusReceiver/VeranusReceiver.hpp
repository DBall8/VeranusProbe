#ifndef VERANUS_RECEIVER_HPP
#define VERANUS_RECEIVER_HPP

#include "drivers/radio/IRadio.hpp"
#include "drivers/uart/IUart.hpp"
#include "drivers/timer/SoftwareTimer.hpp"

#include <stdint.h>

const static uint8_t ID_SIZE = sizeof(uint8_t);

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
    uint8_t probeId = 0;
    float light = 0;
    float temp = 0;
    float humid = 0;
    uint8_t endCode = 0xff;
};

class VeranusReceiver
{
    public:
        VeranusReceiver(const uint8_t* probeIds,
                        const uint16_t numProbes,
                        Radio::IRadio* pRadio,
                        Uart::IUart* pUart,
                        Timer::SoftwareTimer* pTimeoutTimer);
        ~VeranusReceiver();

        void getUpdate(uint8_t probeId);

    private:
        const uint8_t* probeIds_;
        const uint16_t numProbes_;

        Radio::IRadio* pRadio_;
        Uart::IUart* pUart_;
        Timer::SoftwareTimer* pTimeoutTimer_;

        bool request(uint8_t probeId);
        bool receive(uint8_t probeId);
        void transmitOverUart(VeranusData data);
        void transmitFailResponse();

};

#endif