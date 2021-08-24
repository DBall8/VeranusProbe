#ifndef VERANUS_DISPLAY_HPP
#define VERANUS_DISPLAY_HPP

#include "drivers/lcd/ILcd.hpp"
#include "drivers/pwm/IPwm.hpp"


class VeranusDisplay
{
    public:
        VeranusDisplay(Lcd::ILcd* pLcd, Pwm::IPwm* pBrigthnessPwm = nullptr);
        ~VeranusDisplay();

        /**
         * Initialize display and setup all text
         */
        bool setup();

        /**
         * Update the values to display on the display
         * @param   temperatureF    Temperature to display in Fahrenheit
         * @param   humidity        Relative humidity to display
         * @return  True if update was successful
         */
        bool update(float temperatureF, float humidity);

        /**
         * Set which unit the temperature should be displayed in
         * @param   isCelsius   If true, display in Celsius. Otherwise, display in Fahrenheit
         */
        void setTempUnit(bool isCelsius);

        /**
         * Get which unit the temperature is being displayed in
         * @return  If true, displaying in Celsius. Otherwise, displaying in Fahrenheit
         */
        bool isCelsius(){ return isCelsius_; }

        /**
         * Set the brightness of the LCD backlight
         * @param   brightness  Brightness in percent
         */
        void setBrightness(uint8_t brightness);

        /**
         * Get the current brightness of the LCD
         * @return Brightness is whole percents (100% == 100)
         */
        uint8_t getBrightness();

    private:
        Lcd::ILcd* pLcd_;
        Pwm::IPwm* pBrigthnessPwm_;
        int16_t temperature_;   // Last temperature reading 
        uint8_t humidity_;      // Last humidity reading, percent
        uint8_t brightness_;
        bool isCelsius_;

        void clearDisplayBuffer();
};

#endif