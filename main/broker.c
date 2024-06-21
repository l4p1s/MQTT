#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> // Include pthread library
#include "type_h.h"

// Mutex for clients array
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

// 関数プロトタイプ
uint8_t return_str_MSB(uint16_t strlength);
uint8_t return_str_LSB(uint16_t strlength);
unsigned char* encode_Remining_length(int length);
unsigned int decode_remaining_length(unsigned char *encoded_bytes);
unsigned char* return_connack(MQTT_fixed_header *fh);
unsigned char* return_suback(MQTT_fixed_header *fh);
unsigned char* send_publish_command(MQTT_fixed_header *cfh);
void print_struct_values(MQTT_fixed_header *fh, MQTT_payload_header_protocol_name *phpn, MQTT_Variable_Header_in_connect *vh, MQTT_payload_header_in_connect *ph);
void *handle_client(void *arg);
void handle_client_connect(int socket_fd, struct sockaddr_in client_addr, char *client_id);
void handle_client_disconnect(int socket_fd);
void send_message_to_client(int socket_fd, unsigned char *message, int message_length);


void control_topic_subscriber(int socket_fd, struct sockaddr_in client_addr, char *TOPICID , uint8_t request_QoS_level , int message_id){
    for(int i=0 ; i < MAX_CLIENTS ; i++){
        if(subscriber_info[i].socket_fd_for_subscriber == -1){
            subscriber_info[i].socket_fd_for_subscriber = socket_fd;
            subscriber_info[i].client_addr = client_addr;
            subscriber_info[i].cur_message_id = message_id;
            subscriber_info[i].request_QoS_level = request_QoS_level;
            strcpy(subscriber_info[i].client_topic , TOPICID);
            printf("subscribe topic %s \n", subscriber_info[i].client_topic);
            printf("subscribe topic %d \n", subscriber_info[i].request_QoS_level);
            break;
        }
    }
}
void unsubscribe_topic(int socket_fd , char *TOPICID){
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (subscriber_info[i].socket_fd_for_subscriber == socket_fd) {
            subscriber_info[i].socket_fd_for_subscriber = -1;
        }
    }
}

void handle_client_connect(int socket_fd, struct sockaddr_in client_addr, char *client_id ) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd == -1) { // 空いているスロットを見つける
            clients[i].socket_fd = socket_fd;
            clients[i].client_addr = client_addr;
            strncpy(clients[i].client_id, client_id, BUFFER_SIZE - 1);
            clients[i].client_id[BUFFER_SIZE - 1] = '\0'; // 念のためNULL終端
            break;
        }
    }
}

void handle_client_disconnect(int socket_fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd == socket_fd) {
            close(clients[i].socket_fd);
            clients[i].socket_fd = -1;
            memset(clients[i].client_id, 0, BUFFER_SIZE);
            break;
        }
    }
}

void send_message_to_client(int socket_fd, unsigned char *message, int message_length) {
    if (send(socket_fd, message, message_length, 0) < 0) {
        printf("ERROR writing to socket");
    }
}


// uint8_t return_str_MSB(uint16_t strlength) {
//     return (strlength >> 8);
// }

// uint8_t return_str_LSB(uint16_t strlength) {
//     return strlength & 0xFF;
// }

// uint16_t combine_MSB_LSB(uint8_t msb, uint8_t lsb) {
//     return ((uint16_t)msb << 8) | lsb;
// }

// encode.c
// unsigned char* encode_Remining_length(int length) {
//     unsigned char *encoded_bytes = (unsigned char *)malloc(4);
//     if (encoded_bytes == NULL) {
//         fprintf(stderr, "Memory allocation failed\n");
//         return NULL;
//     }

//     int index = 0;
//     do {
//         encoded_bytes[index] = length % 128;
//         length = length / 128;
//         if (length > 0) {
//             encoded_bytes[index] |= 0x80;
//         }
//         index++;
//     } while (length > 0);

//     return encoded_bytes;
// }

// decode.c
// unsigned int decode_remaining_length(unsigned char *encoded_bytes) {
//     unsigned int multiplier = 1;
//     unsigned int value = 0;
//     unsigned char encodedByte;
//     int index = 0;
//     do {
//         encodedByte = encoded_bytes[index++];
//         value += (encodedByte & 127) * multiplier;
//         multiplier *= 128;

//         if (multiplier > 128 * 128 * 128) {
//             fprintf(stderr, "Error: Malformed Remaining Length\n");
//             return 0;
//         }
//     } while ((encodedByte & 128) != 0);

//     return value;
// }

unsigned char* return_connack(MQTT_fixed_header *fh) {
    printf("create connack message\n");
    fh->Control_Packet_type = 0x02; // CONNACK
    fh->Flags = 0x02;
    unsigned char *FX_LENGTH = encode_Remining_length(FIXED_SIZE);
    if (FX_LENGTH == NULL) {
        return NULL;
    }
    memcpy(fh->Remaining_Length, FX_LENGTH, FIXED_SIZE);
    free(FX_LENGTH);

    return (unsigned char *)fh;
}

unsigned char* return_suback(MQTT_fixed_header *fh) {
    printf("create suback message\n");
    fh->Control_Packet_type = 0x09; // SUBACK
    unsigned char *FX_LENGTH = encode_Remining_length(FIXED_SIZE + sizeof(MQTT_payload_message_id_header_in_publish));
    if (FX_LENGTH == NULL) {
        return NULL;
    }
    memcpy(fh->Remaining_Length, FX_LENGTH, FIXED_SIZE);
    free(FX_LENGTH);

    MQTT_payload_message_id_header_in_publish *pmih  = (MQTT_payload_message_id_header_in_publish *)(fh + sizeof(MQTT_fixed_header));
    pmih->MESSAGE_ID_length_MSB = return_str_MSB(MESSAGE_ID_SIZE);
    pmih->MESSAGE_ID_length_LSB = return_str_LSB(MESSAGE_ID_SIZE);
    return (unsigned char *)fh;
}

unsigned char* send_publish_command(MQTT_fixed_header *cfh) {
    cfh->Control_Packet_type = 0x03;
    cfh->Flags = 0x02; 
    unsigned char *FX_LENGTH = encode_Remining_length(FIXED_SIZE);
    if (FX_LENGTH == NULL) {
        return NULL;
    }
    memcpy(cfh->Remaining_Length, FX_LENGTH, FIXED_SIZE);
    free(FX_LENGTH);
    int Remaining_Length_ = sizeof(MQTT_payload_topic_id_header_in_publish) + sizeof(MQTT_payload_message_id_header_in_publish) +  strlen(TOPIC_ID) + MESSAGE_ID_SIZE + FIXED_SIZE;
    MQTT_payload_topic_id_header_in_publish *ptih = (MQTT_payload_topic_id_header_in_publish *)(cfh + sizeof(MQTT_fixed_header) + (Remaining_Length_ / 128) + 1);

    ptih->TOPIC_ID_length_MSB = return_str_MSB(strlen(TOPIC_ID));
    ptih->TOPIC_ID_length_LSB = return_str_LSB(strlen(TOPIC_ID));
    strcpy(ptih->TOPICID, TOPIC_ID);
    
    MQTT_payload_message_id_header_in_publish *pmih = (MQTT_payload_message_id_header_in_publish *)(ptih + sizeof(MQTT_payload_topic_id_header_in_publish)) + strlen(TOPIC_ID);

    pmih->MESSAGE_ID_length_MSB = return_str_MSB(MESSAGE_ID_SIZE);
    pmih->MESSAGE_ID_length_LSB = return_str_LSB(MESSAGE_ID_SIZE);

    MQTT_payload_message *mpm = (MQTT_payload_message *)(pmih + sizeof(MQTT_payload_message_id_header_in_publish));
    memcpy(mpm->message, EXAMPLE_MASSAGE , sizeof(EXAMPLE_MASSAGE));

    return (unsigned char *)cfh;
}

void print_struct_values(MQTT_fixed_header *fh, MQTT_payload_header_protocol_name *phpn, MQTT_Variable_Header_in_connect *vh, MQTT_payload_header_in_connect *ph) {
    printf("MQTT fixed Header in CONNECT:\n");
    printf("Control_Packet_type (Hex): %d\n", fh->Control_Packet_type);
    printf("Message flags from client: %u\n", fh->Flags);
    unsigned int remaining_length = decode_remaining_length(fh->Remaining_Length);
    printf("Decoded Remaining Length: %u\n", remaining_length);

    printf("Protocol Name Length MSB: %d\n", phpn->protocol_name_length_MSB);
    printf("Protocol Name Length LSB: %d\n", phpn->protocol_name_length_LSB);
    printf("Protocol Name: %s\n", phpn->protocol_name);

    printf("Protocol Version: %d\n", vh->protocol_version);
    printf("Something Flags: %d\n", vh->something_flags);
    printf("Keep Alive Timer MSB: %d\n", vh->keep_alive_timer_MSB);
    printf("Keep Alive Timer LSB: %d\n", vh->keep_alive_timer_LSB);

    printf("Client ID Length MSB: %d\n", ph->clientID_length_MSB);
    printf("Client ID Length LSB: %d\n", ph->clientID_length_LSB);
    printf("Client ID: %s\n", ph->clientID);
}

void init_subscriber_info() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        subscriber_info[i].socket_fd_for_subscriber = -1;
    }
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    init_subscriber_info();

    // ソケットの作成
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // アドレスの設定
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // ソケットにアドレスをバインド
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // リスンを開始
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", PORT);

    while (1) {
        // 新しい接続を受け入れ
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        // クライアント情報の保存
        pthread_mutex_lock(&clients_mutex);
        clients[client_count].id = client_count;
        clients[client_count].socket_fd = new_socket;
        clients[client_count].client_addr = address;
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &clients[client_count]);
        client_count++;
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_fd);
    return 0;
}

void *handle_client(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    char buffer[BUFFER_SIZE];
    int valread;

    printf("Client %d connected\n", client->id);

    while ((valread = read(client->socket_fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[valread] = '\0';
        printf("Received message from client %d: %d bytes\n", client->id, valread);

        MQTT_fixed_header *fh = (MQTT_fixed_header *)buffer;
        printf("Control Packet Type: %d\n", fh->Control_Packet_type);
        printf("Flags: %d\n", fh->Flags);
        int remaining_length_ = decode_remaining_length(fh->Remaining_Length);

        switch (fh->Control_Packet_type) {
            case 1: { // CONNECT message
                printf("CONNECT message received from client %d\n", client->id);
                MQTT_payload_header_in_connect *ph = (MQTT_payload_header_in_connect *)(buffer + sizeof(MQTT_fixed_header) + sizeof(MQTT_Variable_Header_in_connect) + sizeof(MQTT_payload_header_protocol_name));
                char client_id[BUFFER_SIZE];
                strncpy(client_id, ph->clientID, BUFFER_SIZE - 1);
                client_id[BUFFER_SIZE - 1] = '\0';

                pthread_mutex_lock(&clients_mutex);
                handle_client_connect(client->socket_fd, client->client_addr, client_id);
                strncpy(client->client_id, client_id, BUFFER_SIZE - 1);
                client->client_id[BUFFER_SIZE - 1] = '\0';
                pthread_mutex_unlock(&clients_mutex);

                MQTT_fixed_header cfh;
                unsigned char *return_connack_packet = return_connack(&cfh);
                if (return_connack_packet == NULL) {
                    fprintf(stderr, "Error creating CONNACK packet\n");
                    close(client->socket_fd);
                    return NULL;
                }
                send_message_to_client(client->socket_fd, return_connack_packet, sizeof(MQTT_fixed_header));
                break;
            }
            case 3: { // PUBLISH message
                printf("PUBLISH message received from client %d\n", client->id);
                break;
            }
            case 8: {
                MQTT_fixed_header cfh;
                printf("SUBSCRIBE message received from client %d\n", client->id);
                MQTT_payload_message_id_header_in_publish *pmih = (MQTT_payload_message_id_header_in_publish *)(fh + sizeof(MQTT_fixed_header) + (remaining_length_ /128)+1 );
                MQTT_payload_topic_id_header_in_subscribe *mptih = (MQTT_payload_topic_id_header_in_subscribe*)(fh + sizeof(MQTT_fixed_header) + sizeof(MQTT_payload_message_id_header_in_publish) + (remaining_length_/128) +1);
                printf("mptih->TOPICID  :  %s\n", mptih->TOPICID);
                control_topic_subscriber(client->socket_fd, client->client_addr, mptih->TOPICID, mptih->request_QoS_level ,combine_MSB_LSB(pmih->MESSAGE_ID_length_MSB , pmih->MESSAGE_ID_length_LSB));
                unsigned char *return_suback_packet=return_suback(&cfh);
                if (return_suback_packet == NULL) {
                    fprintf(stderr, "Error creating SUBNACK packet\n");
                    close(client->socket_fd);
                    return NULL;
                }
                send_message_to_client(client->socket_fd, return_suback_packet, 1024);
                break;
            }
            default:
                fprintf(stderr, "Unsupported Control Packet Type: %d\n", fh->Control_Packet_type);
                continue;
        }
    }

    close(client->socket_fd);
    printf("Client %d disconnected\n", client->id);

    pthread_mutex_lock(&clients_mutex);
    clients[client->id].socket_fd = -1;
    pthread_mutex_unlock(&clients_mutex);

    return NULL;
}