#ifndef VERANUS_DISPLAY_HPP
#define VERANUS_DISPLAY_HPP

#include "drivers/lcd/ILcd.hpp"


class VeranusDisplay
{
    public:
        VeranusDisplay(Lcd::ILcd* pLcd);
        ~VeranusDisplay();

        bool setup();
        bool update(int16_t temperatureF, uint8_t humidity);

    private:
        Lcd::ILcd* pLcd_;
        int16_t temperatureF_;  // Last temperature reading in Fahrenheit
        uint8_t humidity_;      // Last humidity reading, percent

        void clearDisplayBuffer();
};

#endif