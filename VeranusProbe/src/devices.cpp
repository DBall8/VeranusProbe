#include "devices.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/timer/ATmega328/ATmega328Timer.hpp"
#include "drivers/dio/atmega328/Atmega328Dio.hpp"
#include "drivers/uart/atmega328/Atmega328AsynchUart.hpp"
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
#include "drivers/radio/nrf24l01/Nrf24l01.hpp"
#include "drivers/spi/atmega328/Atmega328Spi.hpp"
#include "drivers/pwm/atmega328/Atmega328Pwm.hpp"

using namespace Tic;
using namespace Timer;
using namespace Dio;
using namespace Uart;
using namespace Interrupt;
using namespace Lcd;
using namespace I2c;
using namespace ClimateSensor;
using namespace Adc;
using namespace Filter;
using namespace Radio;
using namespace Spi;
using namespace Pwm;

// CPU Clock frequency
const static uint32_t CPU_CLK = 16000000;

// Set up tic handler
const uint32_t TICS_PER_SECOND = 61u;
static TicCounter ticHandler(TICS_PER_SECOND);
void HandleTicInterrupt()
{
    ticHandler.incrementTicCount();
}

TicCounter* pTicCounter = &ticHandler;

// Set up timer that triggers the tic counter to count
const static TimerPrescaler PRESCALE = PRESCALE_1024;
const static uint16_t TOP = 255;
static Atmega328Timer tmr(Timer::TIMER_2, CTC, PRESCALE, TOP, &HandleTicInterrupt);

// Set up a software timer that decides how often to run the main loop
const static uint32_t CLIMATE_UPDATE_TIME_SECONDS = 60;
const static uint32_t LIGHT_UPDATE_TIME_SECONDS = 15;

static SoftwareTimer updateTimer(1, &ticHandler);
static SoftwareTimer climateTimer(ticHandler.secondsToTics(CLIMATE_UPDATE_TIME_SECONDS), &ticHandler);
static SoftwareTimer lightTimer(ticHandler.secondsToTics(LIGHT_UPDATE_TIME_SECONDS), &ticHandler);

SoftwareTimer* pUpdateTimer = &updateTimer;
SoftwareTimer* pClimateTimer = &climateTimer;
SoftwareTimer* pLightTimer = &lightTimer;

// Interrupt control object, must be enabled on start
static Atmega328Interrupt interruptControl;

// Setup uart for serial debugging
const static uint8_t RX_BUFF_SIZE = 64;
const static uint8_t TX_BUFF_SIZE = 255;
static uint8_t rxBuffer[RX_BUFF_SIZE];
static uint8_t txBuffer[TX_BUFF_SIZE];
static Atmega328AsynchUart serialUart(txBuffer, rxBuffer, TX_BUFF_SIZE, RX_BUFF_SIZE, BaudRate::BAUD_9600, CPU_CLK, &interruptControl);

// Set up IO pins
static Atmega328Dio dataPin0(Port::D, 2, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio dataPin1(Port::D, 3, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio dataPin2(Port::D, 4, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio dataPin3(Port::D, 5, Mode::OUTPUT, Level::L_LOW, false, false);

static Atmega328Dio rsPin(Port::C, 3, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio rwPin(Port::C, 2, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio oePin(Port::C, 1, Mode::OUTPUT, Level::L_LOW, false, false);

static Atmega328Dio radioCePin(Port::D, 6, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio radioCsnPin(Port::D, 7, Mode::OUTPUT, Level::L_LOW, false, false);

// TODO - pin for LCD backlight, add PWM control
static Atmega328Dio lcdBacklightPin(Port::B, 1, Mode::OUTPUT, Level::L_HIGH, false, false);
//static Atmega328Pwm lcdBacklightPwm(Port::B, 1, 50, PwmMode::FAST, PwmResolution::RES_8_BIT);

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

static Dips082Lcd lcd(pRsPin, pRwPin, pOePin, dataPinArray, 4);

static VeranusDisplay display(&lcd);
VeranusDisplay* pDisplay = &display;

static SoftwareTimer i2cTimeoutTimer(2 * TICS_PER_SECOND, &ticHandler);
static Atmega328I2c i2c(I2cBitRate::BR_100_KBPS, &i2cTimeoutTimer);

static Hdc1080ClimateSensor climateSensor(&i2c, &ticHandler);
IClimateSensor* pClimateSensor = &climateSensor;

static LowPassFilter lightSensorFilter(4, 100);
static PhotoTransistor lightSensor(&lightSensorAdc, &lightSensorFilter);
PhotoTransistor* pLightSensor = &lightSensor;

static Atmega328Spi spiDriver(&radioCsnPin, true);
static Nrf24l01 rfTransceiver(&radioCePin, &spiDriver);
IRadio* pRadio = &rfTransceiver;

void initializeDevices()
{
    Delay::Initialize(&ticHandler); // Initialize delay timer

    // Initialize everything that must be initiliazed after construction
    interruptControl.enableInterrupts();    // Enabled interrupts
    tmr.initialize();                       // Start tic tmr
    serialUart.initialize();                // Start serial communication
    PrintHandler::initialize(&serialUart);  // Initialize print handler

    pDisplay->setup();
    if (!pClimateSensor->initialize())
    {
        PRINTLN("Failed to inialize climate sensor.");
    }

    //lcdBacklightPwm.enable();
}