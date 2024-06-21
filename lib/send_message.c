#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

void send_message_to_client(int socket_fd, unsigned char *message, int message_length) {
    if (send(socket_fd, message, message_length, 0) < 0) {
        printf("ERROR writing to socket");
    }
}