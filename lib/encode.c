#include <stdio.h>
#include <stdlib.h>

unsigned char* encode_Remining_length(int length) {
    unsigned char *encoded_bytes = (unsigned char *)malloc(4);
    if (encoded_bytes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    int index = 0;
    do {
        encoded_bytes[index] = length % 128;
        length = length / 128;
        if (length > 0) {
            encoded_bytes[index] |= 0x80;
        }
        index++;
    } while (length > 0);

    return encoded_bytes;
}