#include "devices.hpp"
#include "drivers/timer/TicCounter.hpp"
#include "drivers/timer/ATmega328/ATmega328Timer.hpp"
#include "drivers/dio/atmega328/Atmega328Dio.hpp"
#include "drivers/uart/atmega328/Atmega328AsynchUart.hpp"
#include "drivers/interrupt/atmega328/Atmega328Interrupt.hpp"
#include "utilities/print/Print.hpp"
#include "drivers/timer/Delay.hpp"
#include "drivers/assert/Assert.hpp"
#include "drivers/radio/nrf24l01/Nrf24l01.hpp"
#include "drivers/spi/atmega328/Atmega328Spi.hpp"

using namespace Tic;
using namespace Timer;
using namespace Dio;
using namespace Uart;
using namespace Interrupt;
using namespace Radio;
using namespace Spi;

const static uint16_t TIMEOUT_TIME_SEC = 10;

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
static SoftwareTimer updateTimer(1, &ticHandler);
static SoftwareTimer timeoutTimer(ticHandler.secondsToTics(TIMEOUT_TIME_SEC), &ticHandler);

SoftwareTimer* pUpdateTimer = &updateTimer;

// Interrupt control object, must be enabled on start
static Atmega328Interrupt interruptControl;

// Setup uart for serial debugging
const static uint8_t RX_BUFF_SIZE = 64;
const static uint8_t TX_BUFF_SIZE = 255;
static uint8_t rxBuffer[RX_BUFF_SIZE];
static uint8_t txBuffer[TX_BUFF_SIZE];
static Atmega328AsynchUart serialUart(txBuffer, rxBuffer, TX_BUFF_SIZE, RX_BUFF_SIZE, BaudRate::BAUD_9600, CPU_CLK, &interruptControl);
IUart* pUart = &serialUart;

// Set up IO pins
static Atmega328Dio radioCePin(Port::B, 0, Mode::OUTPUT, Level::L_LOW, false, false);
static Atmega328Dio radioCsnPin(Port::D, 7, Mode::OUTPUT, Level::L_LOW, false, false);

static Atmega328Spi spiDriver(&radioCsnPin, true);
static Nrf24l01 radio(&radioCePin, &spiDriver);

static VeranusReceiver veranusReceiver(&radio,
                                       pUart,
                                       &timeoutTimer);
VeranusReceiver* pVeranusReceiver = &veranusReceiver;

void initializeDevices()
{
    Delay::Initialize(&ticHandler); // Initialize delay timer

    // Initialize everything that must be initiliazed after construction
    interruptControl.enableInterrupts();    // Enabled interrupts
    tmr.initialize();                       // Start tic tmr
    serialUart.initialize();                // Start serial communication
    PrintHandler::initialize(&serialUart);  // Initialize print handler
    pUpdateTimer->enable();                 // Start the update timer

    spiDriver.enable();
    radio.enable();
}