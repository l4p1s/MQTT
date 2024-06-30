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
    int total_sent_packet_length = 0;
    while(total_sent_packet_length < message_length){
        int sent_packet = send(socket_fd, message, message_length, 0);
        total_sent_packet_length += sent_packet;
        printf("total sent packet  : %d\n", total_sent_packet_length);
    }
        printf("send packet\n");
    }
