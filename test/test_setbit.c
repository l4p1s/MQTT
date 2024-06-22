#include <stdio.h>

void setBit(unsigned char *ptr, int i) {
    if (i < 0 || i > 7) {
        perror("Error: bit index out of range\n");
        return ;
    }

    *ptr = (1 << i);
}

int main() {
    unsigned char c = 0x00; // 初期値は0
    int i = 0; // 3番目のビットを1にする

    printf("Before: %02x\n", c);
    setBit(&c, i);
    printf("After:  %02x\n", c);

    return 0;
}