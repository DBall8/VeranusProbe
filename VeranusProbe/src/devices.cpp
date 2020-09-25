#include "devices.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/timer/ATmega328/ATmega328Timer.hpp"
#include "drivers/dio/atmega328/Atmega328Dio.hpp"
#include "drivers/uart/atmega328Uart/Atmega328AsynchUart.hpp"
#include "drivers/interrupt/atmega328/Atmega328Interrupt.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "drivers/lcd/dips082/Dips082Lcd.hpp"
#include "drivers/i2c/atmega328I2c/Atmega328I2c.hpp"
#include "drivers/climateSensor/hdc1080/Hdc1080.hpp"
#include "drivers/assert/Assert.hpp"

using namespace ticCounter;
using namespace timer;
using namespace dio;
using namespace softwareTimer;
using namespace uart;
using namespace interrupt;
using namespace Lcd;
using namespace I2c;
using namespace ClimateSensor;

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
const static uint32_t UPDATE_TIME_SECONDS = 15;
static SoftwareTimer updateTimer(ticHandler.secondsToTics(UPDATE_TIME_SECONDS), &ticHandler);
SoftwareTimer* pUpdateTimer = &updateTimer;

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

void initializeDevices()
{
    Delay::Initialize(&ticHandler); // Initialize delay timer

    // Initialize everything that must be initiliazed after construction
    interruptControl.enableInterrupts();    // Enabled interrupts
    tmr.initialize();                       // Start tic tmr
    serialUart.initialize();                // Start serial communication
    PrintHandler::initialize(&serialUart);  // Initialize print handler
    pUpdateTimer->enable();                 // Start the update timer

    pDisplay->setup();
    if (!pClimateSensor->initialize())
    {
        PRINTLN("Failed to inialize climate sensor.");
    } 
}