#ifndef DISABLE_CLI
#include "ProbeCli.hpp"
#include "devices.hpp"
#include "Settings.hpp"
#include "utilities/print/Print.hpp"
#include "utilities/strings/Strings.hpp"
#include "ProbeStrings.hpp"

using namespace Cli;
using namespace Strings;

const static char* ON_STR = "ON";
const static char* OFF_STR = "OFF";

const static char* WIFI_ENABLED_FORMAT_STR = "WIFI is %s";

static void setLcdBrightness(uint16_t argc, ArgV argv)
{
    if ((argc > 4) ||
        (argc < 1))
    {
        if (settings.debug) PRINTLN(getString(ProbeStrings::INVALID_NUM_PARAMS));
        PRINTLN(getString(ProbeStrings::PASS));
        return;
    }

    // Discover the light mode to set to from the first parameter
    LightMode newLightMode = LightMode::STATIC;
    if (strcompare(argv[1], "D"))
    {
        newLightMode = LightMode::DECREASE;
    }
    else if (strcompare(argv[1], "I"))
    {
        newLightMode = LightMode::INCREASE;
    }

    if ((newLightMode == LightMode::INCREASE) ||
        (newLightMode == LightMode::DECREASE))
    {
        // A dynamic light mode was selected, get the dynamic ranges
        int32_t minLight = str2int(argv[2]);
        int32_t maxLight = str2int(argv[3]);

        // Ensure both values are valid percentages, and max is greater than min
        if ((minLight < 0) || (minLight > 100) ||
            (maxLight < 0) || (maxLight > 100) ||
            (minLight > maxLight))
        {
            if (settings.debug) PRINTLN(getString(ProbeStrings::INVALID_PARAM_VALUE));
            PRINTLN(getString(ProbeStrings::FAIL));
        }
        else
        {
            // Valid values, lets update
            settings.lightMode = newLightMode;
            settings.minLight = minLight;
            settings.maxLight = maxLight;
            PRINTLN(getString(ProbeStrings::PASS));
        }
        
    }
    else
    {
        // Static light mode was selected
        int32_t value = str2int(argv[1]);
        if ((value < 0) || (value > 100))
        {
            if (settings.debug) PRINTLN(getString(ProbeStrings::INVALID_PARAM_VALUE));
            PRINTLN(getString(ProbeStrings::FAIL));
            return;
        }

        settings.lightMode = LightMode::STATIC;
        settings.staticLight = value;
        pDisplay->setBrightness(value);
        PRINTLN(getString(ProbeStrings::PASS));
    }
}

static void wifiCommand(uint16_t argc, ArgV argv)
{
    if (argc == 1)
    {
        PRINTLN(WIFI_ENABLED_FORMAT_STR, settings.wifiEnabled ? ON_STR : OFF_STR);
    }
    else if (strcompare(argv[1], "GET") && (argc == 2))
    {
        char ssid[MAX_SSID_LEN+1];
        if (pWifiInterface->getConfig(ssid, MAX_SSID_LEN+1))
        {
            PRINTLN("Config: %s");
            PRINTLN(getString(ProbeStrings::PASS));
        }
        else
        {
            PRINTLN(getString(ProbeStrings::FAIL));
        }
    }
    else if (strcompare(argv[1], OFF_STR))
    {
        settings.wifiEnabled = false;
        PRINTLN(WIFI_ENABLED_FORMAT_STR, settings.wifiEnabled ? ON_STR : OFF_STR);
    }
    else if (strcompare(argv[1], ON_STR))
    {
        settings.wifiEnabled = true;
        PRINTLN(WIFI_ENABLED_FORMAT_STR, settings.wifiEnabled ? ON_STR : OFF_STR);
    }
    else if (strcompare(argv[1], "SET") && (argc == 4))
    {
        PRINTLN("Config to: %s - %s", argv[2], argv[3]);
        PRINTLN(pWifiInterface->setConfig(argv[2], argv[3]) ? getString(ProbeStrings::PASS) : getString(ProbeStrings::FAIL));
    }
    else
    {
        if (settings.debug) PRINTLN(getString(ProbeStrings::INVALID_NUM_PARAMS));
        PRINTLN(getString(ProbeStrings::FAIL));
    }
}

static void setTempUnit(uint16_t argc, ArgV argv)
{
    if (argc == 1)
    {
        PRINTLN(pDisplay->isCelsius() ? "C" : "F");
    }
    else if (argc == 2)
    {
        if (strcompare(argv[1], "C"))
        {
            pDisplay->setTempUnit(true);
        }
        else if (strcompare(argv[1], "F"))
        {
            pDisplay->setTempUnit(false);
        }
        else
        {
            if (settings.debug) PRINTLN(getString(ProbeStrings::INVALID_PARAM_VALUE));
            PRINTLN(getString(ProbeStrings::FAIL));
            return;
        }
        PRINTLN(getString(ProbeStrings::PASS));
    }
    else
    {
        PRINTLN(getString(ProbeStrings::INVALID_NUM_PARAMS));
    }
}

static void setDebug(uint16_t argc, ArgV argv)
{
    if (argc == 2)
    {
        if (strcompare(argv[1], ON_STR))
        {
            settings.debug = true;
        }
        else if (strcompare(argv[1], OFF_STR))
        {
            settings.debug = false;
        }
        else
        {
            PRINTLN(getString(ProbeStrings::INVALID_PARAM_VALUE));
            return;
        }
    }
    
    if (argc <= 2)
    {
        PRINTLN("DEBUG is %s", settings.debug ? ON_STR : OFF_STR);
    }
    else
    {
        PRINTLN(getString(ProbeStrings::INVALID_PARAM_VALUE));
    }
}

static void getLastReading(uint16_t argc, ArgV argv)
{
    if (argc == 1)
    {
        PRINTLN("T: %f, H: %f, L: %f", latestData.tempF, latestData.humidity, latestData.light);
    }
    else {
        PRINTLN(getString(ProbeStrings::INVALID_NUM_PARAMS));
    }
}

static void idCmd(uint16_t argc, ArgV argv)
{
    if (argc < 3)
    {
        if (argc == 2)
        {
            int32_t value = str2int(argv[1]);
            settings.id = value;
        }

        PRINTLN("ID: %u", settings.id);
        PRINTLN(getString(ProbeStrings::PASS));
    }
    else {
        PRINTLN(getString(ProbeStrings::INVALID_NUM_PARAMS));
        getString(ProbeStrings::FAIL);
    }
}

const static Command commands[] =
{
    {.name = "LIGHT", .function = setLcdBrightness},
    {.name = "WIFI", .function = wifiCommand},
    {.name = "DEBUG", .function = setDebug},
    {.name = "UNIT", .function = setTempUnit},
    {.name = "ECHO", .function = getLastReading},
    {.name = "ID", .function = idCmd}
};
const static uint16_t numCommands = sizeof(commands) / sizeof(commands[0]);

static CommandInterface probeCli(pSerial, commands, numCommands);
CommandInterface* pProbeCli = &probeCli;

#endif