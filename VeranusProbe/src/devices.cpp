#include "devices.hpp"
#include "config.hpp"
#include "Settings.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/timer/ATmega328/ATmega328Timer.hpp"
#include "drivers/dio/atmega328/Atmega328Dio.hpp"
#include "drivers/serial/atmega328/Atmega328AsynchUart.hpp"
#include "drivers/interrupt/atmega328/Atmega328Interrupt.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "drivers/lcd/dips082/Dips082Lcd.hpp"
#include "drivers/i2c/atmega328/Atmega328I2c.hpp"
#include "drivers/climateSensor/hdc1080/Hdc1080.hpp"
#include "drivers/assert/Assert.hpp"
#include "drivers/adc/atmega328/Atmega328Adc.hpp"
#include "drivers/phototransistor/PhotoTransistor.hpp"
#include "utilities/filter/lowPassFilter/LowPassFilter.hpp"
#include "drivers/spi/atmega328/Atmega328Spi.hpp"
#include "drivers/pwm/atmega328/Atmega328Pwm.hpp"
#include "drivers/serial/atmega328/Atmega328SoftwareSerial.hpp"
#include "drivers/watchdog/atmega328/Atmega328Watchdog.hpp"
#include "drivers/eeprom/atmega328/Atmega328Eeprom.hpp"

using namespace Tic;
using namespace Timer;
using namespace Dio;
using namespace SerialComm;
using namespace Interrupt;
using namespace Lcd;
using namespace I2c;
using namespace ClimateSensor;
using namespace Adc;
using namespace Filter;
using namespace Spi;
using namespace Pwm;
using namespace Watchdog;
using namespace Eeprom;

const static uint16_t WDT_TIMEOUT = 2000;
static Atmega328Watchdog wdt;
IWatchdog* pWdt = &wdt;

// Set up tic handler
static TicCounter ticHandler(TICS_PER_SECOND);
void HandleTicInterrupt()
{
    ticHandler.incrementTicCount();
}

// Set up timer that triggers the tic counter to count
const static TimerPrescaler PRESCALE = PRESCALE_1024;
const static uint16_t TOP = 255;
static Atmega328Timer tmr(Timer::TIMER_2, CTC, PRESCALE, TOP, &HandleTicInterrupt);

// Set up a software timers
static SoftwareTimer updateTimer(1, &ticHandler, pWdt);
static SoftwareTimer climateTimer(ticHandler.secondsToTics(CLIMATE_UPDATE_TIME_SECONDS), &ticHandler);
static SoftwareTimer lightTimer(ticHandler.secondsToTics(LIGHT_UPDATE_TIME_SECONDS), &ticHandler);

SoftwareTimer* pUpdateTimer = &updateTimer;
SoftwareTimer* pClimateTimer = &climateTimer;
SoftwareTimer* pLightTimer = &lightTimer;

// Interrupt control object, must be enabled on start
static Atmega328Interrupt interruptControl;

// Setup uart for serial debugging
const static uint8_t RX_BUFF_SIZE = 32;
const static uint8_t TX_BUFF_SIZE = 128;
static uint8_t rxBuffer[RX_BUFF_SIZE];
static uint8_t txBuffer[TX_BUFF_SIZE];
static Atmega328AsynchUart serialUart(txBuffer, rxBuffer, TX_BUFF_SIZE, RX_BUFF_SIZE, BaudRate::BAUD_9600, F_CPU, &interruptControl);
ISerial* pSerial = &serialUart;

// Set up IO pins
static Atmega328Dio dataPin0(Port::D, 2, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio dataPin1(Port::D, 3, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio dataPin2(Port::D, 4, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio dataPin3(Port::D, 5, Mode::OUTPUT, Level::L_LOW, false, false);

static Atmega328Dio rsPin(Port::C, 3, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio rwPin(Port::C, 2, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio oePin(Port::C, 1, Mode::OUTPUT, Level::L_LOW, false, false);

#ifdef WIFI_PROG
static Atmega328Dio wifiTxPin(Port::D, 6, Mode::INPUT, Level::L_LOW, false, false);
#else
static Atmega328Dio wifiTxPin(Port::D, 6, Mode::OUTPUT, Level::L_LOW, false, false);
#endif
static Atmega328Dio wifiRxPin(Port::D, 7, Mode::INPUT, Level::L_LOW, false, true);

// static Atmega328Dio lcdBacklightPin(Port::B, 1, Mode::OUTPUT, Level::L_HIGH, false, false);
const static uint16_t DEFAULT_LCD_BRIGHTNESS = 100;
static Atmega328Pwm lcdBacklightPwm(Port::B, 1, DEFAULT_LCD_BRIGHTNESS, PwmMode::FAST, PwmResolution::RES_8_BIT);

static Atmega328Adc lightSensorAdc(Atmega328Channel::ADC_0, Prescaler::DIV_128, Reference::AREF);

IDio* pRsPin = &rsPin;
IDio* pRwPin = &rwPin;
IDio* pOePin = &oePin;

static IDio* dataPinArray[] =
{
    &dataPin0,
    &dataPin1,
    &dataPin2,
    &dataPin3,
};
IDio** pDataPins = dataPinArray;

uint8_t numPins = sizeof(dataPinArray) / sizeof(dataPinArray[0]);


const static uint16_t WIFI_SERIAL_RX_BUFFER_LEN = 64;
static uint8_t wifiSerialRxBuffer[WIFI_SERIAL_RX_BUFFER_LEN];
static Atmega328SoftwareSerial wifiSerial(&wifiRxPin, &wifiTxPin, &interruptControl, 9600, F_CPU, wifiSerialRxBuffer, WIFI_SERIAL_RX_BUFFER_LEN);
ISerial* pTest = &wifiSerial;

static Dips082Lcd lcd(pRsPin, pRwPin, pOePin, dataPinArray, 4);

static VeranusDisplay display(&lcd, &lcdBacklightPwm);
VeranusDisplay* pDisplay = &display;

static SoftwareTimer i2cTimeoutTimer(ticHandler.secondsToTics(2), &ticHandler, pWdt);
static Atmega328I2c i2c(I2cBitRate::BR_100_KBPS, &i2cTimeoutTimer);

static Hdc1080ClimateSensor climateSensor(&i2c, &ticHandler);

static LowPassFilter lightSensorFilter(4, 100);
static PhotoTransistor lightSensor(&lightSensorAdc, &lightSensorFilter);

static VeranusProbe probe(&climateSensor, &lightSensor);
VeranusProbe* pProbe = &probe;

static SoftwareTimer wifiTimeoutTimer(ticHandler.secondsToTics(WIFI_TIMEOUT_TIME_SECONDS), &ticHandler, &wdt);
static WifiInterface wifiInterface(&wifiSerial, &wifiTimeoutTimer);
WifiInterface* pWifiInterface = &wifiInterface;

static Atmega328Eeprom eepromDriver(&interruptControl);

const static uint16_t numEepromEntries = sizeof(Settings);
static EepromManager eepromManager(&eepromDriver, &settings, numEepromEntries);
EepromManager* pEepromManager = &eepromManager;

VeranusData latestData = 
{
  .tempF = 0,
  .humidity = 0,
  .light = 0
};

void initializeDevices()
{
    pWdt->init();
    pWdt->setTimeout(WDT_TIMEOUT);
    pWdt->enable();

    Delay::Initialize(&ticHandler, &wdt); // Initialize delay timer

    // Initialize everything for printing and timing
    interruptControl.enableInterrupts();    // Enabled interrupts
    tmr.initialize();                       // Start tic tmr
    serialUart.initialize();                // Start serial communication
    PrintHandler::getInstance().initialize(&serialUart);  // Initialize print handler

    ResetCause resetCause = pWdt->getResetCause();
    if (resetCause != ResetCause::POWER_ON)
    {
        PRINTLN("Reset due to %s", (resetCause == ResetCause::WATCHDOG) ?
                                    "watchdog" :
                                    "brown-out");
    }

    // This may take a little while, so nourish the watchdog
    pWdt->reset();

    // Load values from eeprom
    eepromManager.initialize();

    // Initialize settings to defaults if eeprom has changed revision
    if (settings.revision != EEPROM_REV)
    {
        settings.debug = false;
        settings.id = 0xffff;
        settings.lightMode = LightMode::STATIC;
        settings.staticLight = 100;
        settings.minLight = 10;
        settings.maxLight = 100;
        settings.wifiEnabled = true;
        settings.revision = EEPROM_REV;
    }

    // Initialize display and sensors
    pDisplay->setup();

    if (!probe.init())
    {
        PRINTLN("Unable to start probes.");
    }
}