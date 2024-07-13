#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// 仮定する decode_remaining_length 関数
int decode_remaining_length(uint8_t *remaining_length_byte) {
    int multiplier = 1;
    int value = 0;
    uint8_t encodedByte;
    int i = 0;
    do {
        encodedByte = remaining_length_byte[i++];
        value += (encodedByte & 127) * multiplier;
        if (multiplier > 128 * 128 * 128) {
            // エンコードされた長さフィールドがプロトコル制約を超える場合のエラーハンドリング
            return -1;
        }
        multiplier *= 128;
    } while ((encodedByte & 128) != 0);
    return value;
}

// 2進数文字列をバイトに変換する関数
uint8_t binaryStringToByte(const char *binaryString) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte = byte << 1;
        if (binaryString[i] == '1') {
            byte |= 1;
        }
    }
    return byte;
}

int main() {
    // 2進数文字列を定義
    const char *binaryStrings[] = {"10011110", "00011001"};

    // バッファに変換後のバイト列を格納
    char buffer[2];
    for (int i = 0; i < 2; i++) {
        buffer[i] = binaryStringToByte(binaryStrings[i]);
    }

    printf("buffer : %s\n", buffer);

    // 残りの長さを計算するための変数
    int remaining_length_byte_count = 1;
    while (1) {
        if (buffer[remaining_length_byte_count] >> 7 == 1) {
            remaining_length_byte_count += 1;
        } else {
            break;
        }
    }

    uint8_t remaining_length_byte[remaining_length_byte_count];
    memcpy(remaining_length_byte, buffer, remaining_length_byte_count);
    int Packet_Length = decode_remaining_length(remaining_length_byte);

    // 結果を表示
    printf("Remaining Length Byte Count: %d\n", remaining_length_byte_count);
    printf("Packet Length: %d\n", Packet_Length);

    return 0;
}
