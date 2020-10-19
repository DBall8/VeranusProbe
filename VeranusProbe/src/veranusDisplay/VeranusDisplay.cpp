#include "VeranusDisplay.hpp"
#include "drivers/assert/Assert.hpp"
#include "utilities/strings/Strings.hpp"
#include "utilities/Conversions.hpp"

using namespace Lcd;
using namespace Strings;

// Text for temperature and humidity labels
const static char* TEMP_TEXT =  "T:";
const static uint8_t TEMP_TEXT_LEN = strlen(TEMP_TEXT);

const static char* HUMID_TEXT = "H:";
const static uint8_t HUMID_TEXT_LEN = strlen(HUMID_TEXT);

const static char* TEMP_UNIT = "F";
const static char* HUMID_UNIT = "%";
const static uint8_t UNIT_LEN = 1;

const static uint8_t DEGREE_CHAR_CODE = 0xDF;

// Indexes for text location
const static uint8_t TEMP_ROW = 0;
const static uint8_t HUMID_ROW = 1;

const static uint8_t TEMP_VALUE_INDEX = 2;
const static uint8_t HUMID_VALUE_INDEX = 3;
const static uint8_t TEMP_UNIT_INDEX = 7;
const static uint8_t HUMID_UNIT_INDEX = 6;

const static uint8_t DEGREE_INDEX = 6;

const static uint8_t TEMP_VALUE_LEN = 4;
const static uint8_t HUMID_VALUE_LEN = 3;
static char stringBuffer[TEMP_VALUE_LEN + 1];
static char displayBuffer[TEMP_VALUE_LEN];

VeranusDisplay::VeranusDisplay(Lcd::ILcd* pLcd):
    pLcd_(pLcd),
    temperatureF_(0),
    humidity_(0)
{
    assert(pLcd_ != nullptr);
    stringBuffer[TEMP_VALUE_LEN] = '\0';
}

VeranusDisplay::~VeranusDisplay()
{

}

bool VeranusDisplay::setup()
{
    pLcd_->initialize();

    // Temperature label
    pLcd_->display(TEMP_TEXT, TEMP_TEXT_LEN);

    // Degree symbol
    pLcd_->setCursor(TEMP_ROW, DEGREE_INDEX);
    pLcd_->display(DEGREE_CHAR_CODE);

    // Temperature unit
    pLcd_->display(TEMP_UNIT, UNIT_LEN);

    // Humidity label
    pLcd_->setCursor(HUMID_ROW, 0);
    pLcd_->display(HUMID_TEXT, HUMID_TEXT_LEN);

    // Humidity unit
    pLcd_->setCursor(HUMID_ROW, HUMID_UNIT_INDEX);
    pLcd_->display(HUMID_UNIT, UNIT_LEN);
}

bool VeranusDisplay::update(float temperatureF, float humidity)
{
    // Round to nearest whole value
    temperatureF = round(temperatureF);
    humidity = round(humidity);

    if (temperatureF_ != temperatureF)
    {
        temperatureF_ = temperatureF;
        clearDisplayBuffer();

        // Fill display buffer with string representation of temperature
        // Right align
        int2str(temperatureF, stringBuffer, TEMP_VALUE_LEN);
        uint8_t tempStrLen = strlen(stringBuffer);
        uint8_t offset = TEMP_VALUE_LEN - tempStrLen;
        copy(&(displayBuffer[offset]), stringBuffer, tempStrLen);

        // Display buffer contents in the temperature slot
        pLcd_->setCursor(TEMP_ROW, TEMP_VALUE_INDEX);
        pLcd_->display(displayBuffer, TEMP_VALUE_LEN);
    }

    if (humidity_ != humidity)
    {
        humidity_ = humidity;

        clearDisplayBuffer();

        // Fill display buffer with string representation of humidity
        // Right align
        int2str(humidity, stringBuffer, HUMID_VALUE_LEN);
        uint8_t humidStrLen = strlen(stringBuffer);
        uint8_t offset = HUMID_VALUE_LEN - humidStrLen;
        copy(&(displayBuffer[offset]), stringBuffer, humidStrLen);

        // Display buffer contents in humidity slot
        pLcd_->setCursor(HUMID_ROW, HUMID_VALUE_INDEX);
        pLcd_->display(displayBuffer, HUMID_VALUE_LEN);
    }

    return true;
}

void VeranusDisplay::clearDisplayBuffer()
{
    for (uint8_t i=0; i<TEMP_VALUE_LEN; i++)
    {
        displayBuffer[i] = ' ';
    }
}