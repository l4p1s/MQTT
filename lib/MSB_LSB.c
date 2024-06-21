#include <stdio.h>
#include <stdint.h>

uint8_t return_str_MSB(uint16_t strlength) {
    return (strlength >> 8);
}

uint8_t return_str_LSB(uint16_t strlength) {
    return strlength & 0xFF;
}

uint16_t combine_MSB_LSB(uint8_t msb, uint8_t lsb) {
    return ((uint16_t)msb << 8) | lsb;
}