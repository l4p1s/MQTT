#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

int main() {
    size_t size;
    printf("Enter the number of bytes to allocate: ");
    scanf("%zu", &size);

    // メモリを確保
    unsigned char *memory = (unsigned char *)malloc(size);
    if (memory == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    printf("size of malloced memory  : %zd\n", malloc_usable_size(memory));

    // メモリをゼロで初期化
    memset(memory, 0, size);

    // メモリにデータを書き込む（例として0から順にバイト値を設定）
    for (size_t i = 0; i < size; ++i) {
        memory[i] = (unsigned char)i;
    }

    // メモリ内容を表示
    printf("Memory contents:\n");
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", memory[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // メモリを解放
    free(memory);

    return 0;
}
