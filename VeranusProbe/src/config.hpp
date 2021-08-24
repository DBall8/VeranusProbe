#ifndef CONFIG_HPP
#define CONFIG_HPP

const static uint32_t TICS_PER_SECOND = 61u;

const static uint32_t CLIMATE_UPDATE_TIME_SECONDS = 5 * 60;
const static uint32_t LIGHT_UPDATE_TIME_SECONDS = 15;
const static uint32_t WIFI_SUCCESS_UPDATE_READINGS = 3;
const static uint32_t WIFI_FAIL_UPDATE_TIME_SECONDS = 1;
const static uint32_t WIFI_TIMEOUT_TIME_SECONDS = 1 * 60;

// Max and minimum values for scaling brightness based on light level
const static uint8_t MAX_LIGHT_SCALE = 90;
const static uint8_t MIN_LIGHT_SCALE = 10;

// Reading error correction
// V1
// const static float TEMP_CORRECTION_SLOPE = 0.168f;
// const static float TEMP_CORRECTION_OFFSET = -16.0f;

// // V2
// const static float TEMP_CORRECTION_SLOPE = 0.075f;
// const static float TEMP_CORRECTION_OFFSET = -11.4f;

// const static uint8_t CORRECT_GRADIENT_PERIOD = 6;

#endif