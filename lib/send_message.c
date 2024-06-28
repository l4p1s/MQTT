#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

void print_bits2(const char *label, const unsigned char *data, int length) {
    printf("%s: ", label);
    for (int i = 0; i < length; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (data[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

void send_message_to_client(int socket_fd, unsigned char *message, int message_length) {
    print_bits2("send packet contents" , message , message_length);
    if (send(socket_fd, message, message_length, 0) < 0) {
        printf("ERROR writing to socket");
    }else{
        printf("send packet\n");
    }
}