#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <stdint.h>

const static uint8_t EEPROM_REV = 2;

enum LightMode : uint8_t
{
    STATIC,
    INCREASE,
    DECREASE
};

struct Settings
{
    uint16_t id;
    uint8_t revision;
    LightMode lightMode;
    uint8_t staticLight;
    uint8_t minLight;
    uint8_t maxLight;
    bool debug;
    bool wifiEnabled;
};

extern Settings settings;

#endif