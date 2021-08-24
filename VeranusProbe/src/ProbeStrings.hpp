#ifndef PROBE_STRINGS_HPP
#define PROBE_STRINGS_HPP

#include <stdint.h>
#include <avr/pgmspace.h>

enum class ProbeStrings : uint8_t
{
    INVALID_PARAM_VALUE = 0,
    INVALID_NUM_PARAMS,
    CLIMATE_SENSOR_FAILURE,
    PASS,
    FAIL
};

const static uint8_t MAX_STRING_LEN = 64;
static char stringBuffer[MAX_STRING_LEN];

const PROGMEM char invalidParamValueStr[] = "Invalid parameter value.";
const PROGMEM char invalidNumParamsStr[] = "Invalid number of parameters.";
const PROGMEM char climateSensorFailure[] = "Failed to read from climate sensor.";
const PROGMEM char pass[] = "PASS";
const PROGMEM char fail[] = "FAIL";
const char* const stringTable[] PROGMEM = {invalidParamValueStr, invalidNumParamsStr, climateSensorFailure, pass, fail};

static const char* getString(ProbeStrings probeString)
{
    strcpy_P(stringBuffer, (char *)pgm_read_word(&(stringTable[(uint8_t)probeString])));
    return stringBuffer;
}

#endif