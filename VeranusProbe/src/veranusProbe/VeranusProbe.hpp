#ifndef VERANUS_PROBE_HPP
#define VERANUS_PROBE_HPP

#include "drivers/climateSensor/IClimateSensor.hpp"
#include "drivers/phototransistor/PhotoTransistor.hpp"

class VeranusProbe
{
    public:
        VeranusProbe(ClimateSensor::IClimateSensor* pClimateSensor,
                     PhotoTransistor* pLightSensor);

        ~VeranusProbe(){}

        bool init();
        void setLcdBrightness(float lcdBrightness);
        bool readClimate(float& temperatureF, float& humidity);
        bool readLight(float& light);

    private:
        ClimateSensor::IClimateSensor* pClimateSensor_;
        PhotoTransistor* pLightSensor_;
        float lcdBrightness_;
        uint8_t numClimateReadings_;

        float getTemperatureCorrected(float temperatureF);
        float getHumidityCorrected(float humidity);
        float getHumidityCorrected(float humidity, float tempMeasured, float tempCorrected);
};

#endif