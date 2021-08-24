#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "utilities/Conversions.hpp"
#include "drivers/timer/Delay.hpp"
#include "version.hpp"
#include "Settings.hpp"
#include "config.hpp"
#include "ProbeStrings.hpp"

#ifndef DISABLE_CLI
#include "ProbeCli.hpp"
#endif

extern "C" void __cxa_pure_virtual() { while (1); }

// Count used to sync wifi updates to be directly after climate readings
static int8_t readingsUntilUpdate = 1;

void updateReceiver();
void updateLightSensor();
void updateClimateSensor();
void updateWifi();
void loop();

int main(void)
{
    initializeDevices();

    PRINTLN("Build %d.%d", V_MAJOR, V_MINOR);
    PRINTLN("ID: %d", settings.id);

    // Get initial readings for each sensor
    updateClimateSensor();
    updateLightSensor();

    // Start the update timers
    pUpdateTimer->enable();
    pClimateTimer->enable();
    pLightTimer->enable();

#ifndef DISABLE_CLI
    // Enable our CLI
    pProbeCli->enable();
#endif

#ifndef WIFI_PROG
    // Intialize wifi interface
    pWifiInterface->init();
#else
    settings.wifiEnabled = false;
#endif

    for (;;) {

        if (pUpdateTimer->hasPeriodPassed())
        {
            loop();
        }
    }

  return 0;
}

void loop()
{
#ifndef DISABLE_CLI
    // Update the cli
    pProbeCli->update();
#endif

    // If enough time has elapsed, update the climate sensor data
    if (pClimateTimer->hasPeriodPassed())
    {
        updateClimateSensor();

        // Reset timer to ensure we do not try and read the sensor
        // again before it is ready
        pClimateTimer->reset();
    }

    // If enough time has passed, update the light sensor
    if (pLightTimer->hasPeriodPassed())
    {
        updateLightSensor();
    }

    // Update wifi driver
    if (settings.wifiEnabled)
    {
        updateWifi();
    }

    // Save any changes to eeprom
    pEepromManager->update();
}

void updateClimateSensor()
{
    // Try and get a new climate reading
    if (pProbe->readClimate(latestData.tempF, latestData.humidity))
    {

#ifdef CLIMATE_DEBUG
        PRINTLN("T,%f,H,%f", latestData.tempF, latestData.humidity);
#else
        if (settings.debug)
        {
            PRINTLN("T,%f,H,%f", latestData.tempF, latestData.humidity);
        }
#endif

        // Update display
        pDisplay->update(latestData.tempF, latestData.humidity);

        // Decrease the number of climate readings required to update the wifi module
        readingsUntilUpdate--;

        // Let the probe know what the brightness of the LCD is so that we can adjust accordingly
        pProbe->setLcdBrightness(static_cast<float>(pDisplay->getBrightness()));
    }
    else
    {
        if (settings.debug) PRINTLN(getString(ProbeStrings::CLIMATE_SENSOR_FAILURE));
    }
}

void updateLightSensor()
{
    pProbe->readLight(latestData.light);
#ifndef CLIMATE_DEBUG
    if (settings.debug) PRINTLN("L,%d", (uint16_t)latestData.light);
#endif

    if (settings.lightMode == LightMode::INCREASE)
    {
        if (settings.maxLight > settings.minLight)
        {
            // Calculate the desired brightness for the increase mode
            float brightness = 0;
            if (latestData.light <= MIN_LIGHT_SCALE)
            {
                brightness = settings.minLight;
            }
            else if (latestData.light >= MAX_LIGHT_SCALE)
            {
                brightness = settings.maxLight;
            }
            else 
            {
                float brightnessPercent = (latestData.light - MIN_LIGHT_SCALE) / (MAX_LIGHT_SCALE - MIN_LIGHT_SCALE);
                brightness = (brightnessPercent * (settings.maxLight - settings.minLight)) + settings.minLight;
            }
            pDisplay->setBrightness(static_cast<uint8_t>(brightness));
        }
    }
    else if (settings.lightMode == LightMode::DECREASE)
    {
        if (settings.maxLight > settings.minLight)
        {
            float brightness = 0;
            if (latestData.light <= MIN_LIGHT_SCALE)
            {
                brightness = settings.maxLight;
            }
            else if (latestData.light >= MAX_LIGHT_SCALE)
            {
                brightness = settings.minLight;
            }
            else 
            {
                float brightnessPercent = (MAX_LIGHT_SCALE - latestData.light) / (MAX_LIGHT_SCALE - MIN_LIGHT_SCALE);
                brightness = (brightnessPercent * (settings.maxLight - settings.minLight)) + settings.minLight;
            }
            pDisplay->setBrightness(static_cast<uint8_t>(brightness));
        }
    }
}

void updateWifi()
{
    switch (pWifiInterface->getState())
    {
        case WifiState::IDLE:
        {
            if (readingsUntilUpdate <= 0)
            {
                pWifiInterface->send(settings.id, latestData.tempF, latestData.humidity, latestData.light);
            }
            break;
        }

        case WifiState::PENDING:
        {
            // Transaction pending, let the wifi interface do its thing
            pWifiInterface->update();
            break;
        }

        case WifiState::COMPLETED:
        {
            // Set the number of climates readings to wait for before sending to wifi again
            // based on whether or not this transmission succeeded
            bool success = pWifiInterface->getSuccess();
            readingsUntilUpdate = success ?
                                    WIFI_SUCCESS_UPDATE_READINGS :
                                    WIFI_FAIL_UPDATE_TIME_SECONDS;

            if (settings.debug) PRINTLN("Send %s", success ? getString(ProbeStrings::PASS) : getString(ProbeStrings::FAIL));
            break;
        }

        default:
        {
            // Still active, do nothing
            break;
        }
    }
}