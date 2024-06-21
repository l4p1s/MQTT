#include <stdio.h>

unsigned int decode_remaining_length(unsigned char *encoded_bytes) {
    unsigned int multiplier = 1;
    unsigned int value = 0;
    unsigned char encodedByte;
    int index = 0;
    do {
        encodedByte = encoded_bytes[index++];
        value += (encodedByte & 127) * multiplier;
        multiplier *= 128;

        if (multiplier > 128 * 128 * 128) {
            fprintf(stderr, "Error: Malformed Remaining Length\n");
            return 0;
        }
    } while ((encodedByte & 128) != 0);

    return value;
}