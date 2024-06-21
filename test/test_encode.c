#include <stdio.h>
#include <stdlib.h>

// エンコード関数
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

// デコード関数
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

// ビット表示関数
void print_bits(const char *label, const unsigned char *data, int length) {
    printf("%s: ", label);
    for (int i = 0; i < length; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (data[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

int main() {
    int length = 300; // エンコードする長さの例として300を使用

    // エンコードして結果を取得
    unsigned char *encoded = encode_Remining_length(length);
    if (encoded == NULL) {
        return 1; // エンコード失敗時の処理
    }

    // ビット表示関数を使って表示
    print_bits("Encoded Length", encoded, 4);

    // デコードして元の長さを取得
    unsigned int decoded_length = decode_remaining_length(encoded);

    // 元の長さを表示
    printf("Decoded Length: %u\n", decoded_length);

    // メモリの解放
    free(encoded);

    return 0;
}
