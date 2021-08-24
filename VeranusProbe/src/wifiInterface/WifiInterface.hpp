#ifndef WIFI_INTERFACE_HPP
#define WIFI_INTERFACE_HPP

#include "VeranusWifiCodes.hpp"
#include "drivers/serial/ISerial.hpp"
#include "drivers/timer/SoftwareTimer.hpp"

const static uint8_t MAX_SSID_LEN = 32;

enum WifiState : uint8_t
{
    IDLE,
    PENDING,
    COMPLETED
};

class WifiInterface
{
    public:
        WifiInterface(SerialComm::ISerial* pSerial,
                      Timer::SoftwareTimer* pTimeoutTimer);

        void init();
        void update();
        void send(uint16_t probeId, float temperature, float humidity, float light);
        bool setConfig(const char* ssid, const char* password);
        bool getConfig(char* ssid, uint16_t maxLength);

        WifiState getState();
        bool getSuccess();

    private:
        SerialComm::ISerial* pSerial_;
        Timer::SoftwareTimer* pTimeoutTimer_;

        const static uint8_t VAL_BUFFER_LEN = MAX_SSID_LEN + 2;
        char valBuffer_[VAL_BUFFER_LEN];

        uint8_t bufferIndex_;
        WifiState state_;
        VeranusWifiCode activeTransaction_;
        bool success_;

        void startNewCommand(VeranusWifiCode commandCode);
        bool isBusy();
        bool checkReponse(char*& response);

        bool isSuccess(uint8_t response);
        VeranusWifiCode getCode(uint8_t response);
};

#endif