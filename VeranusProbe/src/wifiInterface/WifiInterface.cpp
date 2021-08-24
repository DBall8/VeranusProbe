#include "WifiInterface.hpp"
#include "utilities/strings/Strings.hpp"
#include "Settings.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/assert/Assert.hpp"
#include "config.hpp"
#include "drivers/timer/Delay.hpp"

using namespace SerialComm;
using namespace Strings;
using namespace Timer;
using namespace Watchdog;

const static char NEWLINE[] = "\r\n";
const static uint8_t NEWLINE_LEN = sizeof(NEWLINE) - 1;

const static char DELIM = ' ';

const static char DATA_STR[] = "SD ";
const static uint8_t DATA_STR_LEN = sizeof(DATA_STR) - 1;
const static uint8_t FLOAT_DECIMAL_PLACES = 4;

const static char SET_CONFIG_STR[] = "SC";
const static uint8_t SET_CONFIG_STR_LEN = sizeof(SET_CONFIG_STR) - 1;

const static char GET_CONFIG_STR[] = "GC";
const static uint8_t GET_CONFIG_STR_LEN = sizeof(GET_CONFIG_STR) - 1;

const static char PASS[] = "PASS";
const static char FAIL[] = "FAIL";
const static uint8_t EXPECTED_RESPONSE_LEN = 4;

WifiInterface::WifiInterface(ISerial* pSerial, 
                             SoftwareTimer* pTimeoutTimer):
    pSerial_(pSerial),
    pTimeoutTimer_(pTimeoutTimer),
    bufferIndex_(0),
    state_(WifiState::IDLE),
    activeTransaction_(VeranusWifiCode::NONE),
    success_(false)
{
}

void WifiInterface::init()
{
    pSerial_->initialize();
    DELAY(1000);
    pSerial_->write(NEWLINE, NEWLINE_LEN);
}

void WifiInterface::update()
{
    if (state_ == WifiState::PENDING)
    {
        // Check for any response on the CLI
        char* responseStr;
        if (checkReponse(responseStr))
        {
            // Check if the response matches the transaction we are waiting for
            uint8_t response = str2int(responseStr);
            if (getCode(response) == activeTransaction_)
            {
                pTimeoutTimer_->disable();

                // This code is for this request, check if we got a success response
                success_ = isSuccess(response);
                state_ = WifiState::COMPLETED;

                activeTransaction_ = VeranusWifiCode::NONE;
            }
        }
        // Check if connecting has timed out
        else if (pTimeoutTimer_->hasOneShotPassed())
        {
            pTimeoutTimer_->disable();

            success_ = false;
            activeTransaction_ = VeranusWifiCode::NONE;
            state_ = WifiState::COMPLETED;
        }
    }
}

void WifiInterface::send(uint16_t probeId, float temperature, float humidity, float light)
{
    startNewCommand(VeranusWifiCode::SEND);

    // Write command name
    pSerial_->write(DATA_STR, DATA_STR_LEN);

    // Write probe ID
    int2str(probeId, valBuffer_, VAL_BUFFER_LEN);
    pSerial_->write(valBuffer_, strlen(valBuffer_));
    pSerial_->write(&DELIM, sizeof(char));

    // Write temperature
    float2str(temperature, valBuffer_, VAL_BUFFER_LEN, FLOAT_DECIMAL_PLACES);
    pSerial_->write(valBuffer_, strlen(valBuffer_));
    pSerial_->write(&DELIM, sizeof(char));

    // Write humidity
    float2str(humidity, valBuffer_, VAL_BUFFER_LEN, FLOAT_DECIMAL_PLACES);
    pSerial_->write(valBuffer_, strlen(valBuffer_));
    pSerial_->write(&DELIM, sizeof(char));

    // Write light
    float2str(light, valBuffer_, VAL_BUFFER_LEN, FLOAT_DECIMAL_PLACES);
    pSerial_->write(valBuffer_, strlen(valBuffer_));

    // Send command
    pSerial_->write(NEWLINE, NEWLINE_LEN);

    // Go to sending state and start timeout
    state_ = WifiState::PENDING;
    pTimeoutTimer_->enable();
    if (settings.debug) PRINTLN("Sending...");
}

bool WifiInterface::checkReponse(char*& response)
{
    response = nullptr;

    while (pSerial_->isDataAvailable())
    {
        char value;
        pSerial_->read(&value, 1);

        if (value == '\n')
        {
            // End of line found, return the value retrieved
            valBuffer_[bufferIndex_] = '\0';
            bufferIndex_ = 0;

            response = valBuffer_;
            return true;
        }
        else if ((bufferIndex_ < (VAL_BUFFER_LEN - 1)) &&
                 (value != '\r'))
        {
            // Ignore carriage returns
            valBuffer_[bufferIndex_] = value;
            bufferIndex_++;
        }
    }

    // Full line not received yet
    return false;
}

bool WifiInterface::isSuccess(uint8_t response)
{
    return (response & SUCCESS_BITMASK);
}

VeranusWifiCode WifiInterface::getCode(uint8_t response)
{
    // Return the response with the success bit ignored
    return (VeranusWifiCode)(response & ~(SUCCESS_BITMASK));
}

bool WifiInterface::setConfig(const char* ssid, const char* password)
{
    startNewCommand(VeranusWifiCode::SET_CONFIG);

    // Write set config command
    pSerial_->write(SET_CONFIG_STR, SET_CONFIG_STR_LEN);
    pSerial_->write(&DELIM, sizeof(char));

    // Write SSID
    pSerial_->write(ssid, strlen(ssid));
    pSerial_->write(&DELIM, sizeof(char));

    // Write password
    pSerial_->write(password, strlen(password));
    pSerial_->write(NEWLINE, NEWLINE_LEN);

    // Go to sending state and start timeout
    state_ = WifiState::PENDING;
    pTimeoutTimer_->enable();
    

    // Wait until response or timeout
    while (isBusy())
    {
        update();
    }

    // Get the result of the command, and return to Idle
    return getSuccess();
}

bool WifiInterface::getConfig(char* ssid, uint16_t maxLength)
{
    startNewCommand(VeranusWifiCode::GET_CONFIG);

    // Write set config command
    pSerial_->write(GET_CONFIG_STR, GET_CONFIG_STR_LEN);
    pSerial_->write(NEWLINE, NEWLINE_LEN);

    // Go to sending state and start timeout
    state_ = WifiState::PENDING;
    pTimeoutTimer_->enable();

    // First, await a text response
    char* response;
    while (!checkReponse(response))
    {
        if (pTimeoutTimer_->hasOneShotPassed())
        {
            pTimeoutTimer_->disable();
            return false;
        }
    }

    // Received a code, not an SSID
    if (getCode(str2int(response)) == VeranusWifiCode::GET_CONFIG)
    {
        return false;
    }

    // Wasn't a code, so assume ssid
    strncpy(ssid, response, maxLength);

    // Await the success/fail message
    while (isBusy())
    {
        update();
    }

    return getSuccess();
}

void WifiInterface::startNewCommand(VeranusWifiCode commandCode)
{
    // Wait until no other transaction is in progress
    while (isBusy())
    {
        update();
    }

    pSerial_->flush();
    bufferIndex_ = 0;
    activeTransaction_ = commandCode;
}

bool WifiInterface::isBusy()
{
    return (state_ == WifiState::PENDING);
}

WifiState WifiInterface::getState(){ return state_; }

bool WifiInterface::getSuccess()
{
    bool success = false;
    if (state_ == WifiState::COMPLETED)
    {
        success = success_;
        success_ = false;
        state_ = WifiState::IDLE;
    }

    return success;
}