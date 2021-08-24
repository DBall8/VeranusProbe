#include "veranusProbe/VeranusProbe.hpp"
#include "utilities/print/Print.hpp"
#include "config.hpp"
#include "Settings.hpp"

#include <math.h>

const static float A_VAL = 17.27f;
const static float B_VAL = 237.7f;

// V3
const static float TEMP_CORRECTION_SLOPE = 0.0f;
const static float TEMP_CORRECTION_OFFSET = -3.0f;
const static float TEMP_CORRECTION_LCD_FACTOR = -2.0f;

const static float HUM_CORRECTION_SLOPE = 0.0f;//0.4f;
const static float HUM_CORRECTION_OFFSET = 0;//-7.75f;
const static float HUM_CORRECTION_LCD_FACTOR = -4.5f;

const static uint8_t CORRECT_GRADIENT_PERIOD = 6;

const static float MAX_LED_BRIGHTNESS = 100.f;

VeranusProbe::VeranusProbe(ClimateSensor::IClimateSensor* pClimateSensor,
                           PhotoTransistor* pLightSensor):
    pClimateSensor_(pClimateSensor),
    pLightSensor_(pLightSensor),
    lcdBrightness_(0),
    numClimateReadings_(0)
{
}

bool VeranusProbe::init()
{
    if (!pClimateSensor_->initialize())
    {
        return false;
    }

    return true;
}

void VeranusProbe::setLcdBrightness(float lcdBrightness)
{
    lcdBrightness_ = lcdBrightness;
}

bool VeranusProbe::readClimate(float& temperatureF, float& humidity)
{
    if (!pClimateSensor_->update())
    {
        return false;
    }
  
    // Get the readings from the probe
    float tempMeasured = pClimateSensor_->getTemperatureFahrenheit();
    float humidityMeasured = pClimateSensor_->getRelativeHumidity();

#ifdef CLIMATE_DEBUG
    PRINT("MT,%f,MH,%f,", tempMeasured, humidityMeasured);
#endif

    // Apply corrections to account for heat from the board and its casing
    temperatureF = getTemperatureCorrected(tempMeasured);
    humidity = getHumidityCorrected(humidityMeasured, tempMeasured, temperatureF);

    return true;
}

bool VeranusProbe::readLight(float& light)
{
    pLightSensor_->update();
    light = pLightSensor_->getLightPercent();
    return true;
}

float VeranusProbe::getTemperatureCorrected(float temperatureF)
{
    /*
     * Correct for temperature error due to self heating
     * Correct linear from offsets from real values measured over a range of temperatures
     * However, since self heating occurs over the first few minutes, only apply the correction 
     * partially at first, and then ramp up to the full correction
     */
    float tempAdjustment = (temperatureF * TEMP_CORRECTION_SLOPE) +
                           TEMP_CORRECTION_OFFSET;

    // LCD heat is exponentially related to the voltage (brightness)
    // (brightness^2) / (100%^2) * LCD temperature increase
    tempAdjustment += (lcdBrightness_ / MAX_LED_BRIGHTNESS) * TEMP_CORRECTION_LCD_FACTOR;

    // Self heating occurs gradually at startup to apply it gradually
    if (numClimateReadings_ < CORRECT_GRADIENT_PERIOD)
    {
        tempAdjustment *= (float)numClimateReadings_ / CORRECT_GRADIENT_PERIOD;
        numClimateReadings_++; 
    }

    // Only adjust temperature down, since we are only concerned
    // about self heating, not self cooling!
    if (tempAdjustment < 0) temperatureF += tempAdjustment;
    return temperatureF;
}

float VeranusProbe::getHumidityCorrected(float humidity)
{
        /*
     * Correct for humidity error due to self heating
     * Correct linear from offsets from real values measured over a range of temperatures
     * However, since self heating occurs over the first few minutes, only apply the correction 
     * partially at first, and then ramp up to the full correction
     */
    float humAdjustment = (humidity * HUM_CORRECTION_SLOPE) +
                           HUM_CORRECTION_OFFSET;

    // LCD heat is exponentially related to the voltage (brightness)
    // (brightness^2) / (100%^2) * LCD temperature increase
    humAdjustment += (lcdBrightness_  / MAX_LED_BRIGHTNESS) * HUM_CORRECTION_LCD_FACTOR;

    // Self heating occurs gradually at startup to apply it gradually
    if (numClimateReadings_ < CORRECT_GRADIENT_PERIOD)
    {
        humAdjustment *= (float)numClimateReadings_ / CORRECT_GRADIENT_PERIOD;
        numClimateReadings_++; 
    }

    humidity += humAdjustment;
    return humidity;
}

float VeranusProbe::getHumidityCorrected(float humidity, float tempMeasured, float tempCorrected)
{
    /**
     * Humidity correction is based on the dewpoint equation found here: http://hyperphysics.phy-astr.gsu.edu/hbase/Kinetic/relhum.html
     * Basically, the dewpoint should be the same at the actual temperature and humidity, as at the measured temperature and humidity
     * Therefore, the equation for dewpoint with the measured values equals the equation with corrected values.
     * Solve this equation for correct humidity, since we know both measured values, and have a corrected temperature.
     * 
     * Hcorr = Hmeas * e^(  (a*b*(Tmeas - Tcorr)) / ((b + Tmeas) * (b +Tcorr))  )  
     * where a and b are given constants
     */
    float tempPortion = A_VAL * B_VAL * (tempMeasured - tempCorrected);
    tempPortion /= (B_VAL + tempMeasured) * (B_VAL + tempCorrected);
    float hTempCorrected = humidity * exp(tempPortion);

    return getHumidityCorrected(hTempCorrected);
}