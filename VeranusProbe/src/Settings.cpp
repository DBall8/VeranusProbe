#include "Settings.hpp"

Settings settings =
{
    .id = 0,
    .revision = 0,
    .lightMode = LightMode::STATIC,
    .staticLight = 100,
    .minLight = 10,
    .maxLight = 10,
    .debug = true,
    .wifiEnabled = true
};

// Ensure this whole struct fits in EEPROM
static_assert(sizeof(settings) < 1024, "Too many values to fit in EEPROM");