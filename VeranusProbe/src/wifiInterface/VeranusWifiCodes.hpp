#ifndef VERANUS_WIFI_CODES
#define VERANUS_WIFI_CODES

#include <stdint.h>

const static uint8_t SUCCESS_BITMASK = 0x80;
enum VeranusWifiCode : uint8_t
{
    NONE = 0x00,
    CONNECT = 0x01,
    DISCONNECT = 0x02,
    SET_CONFIG = 0x03,
    GET_CONFIG = 0x04,
    TEST = 0x05,
    SEND = 0x06,
    INVALID
};

#endif