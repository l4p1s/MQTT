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
#include "../lib/build_command.h"
#include "../lib/encode.h"
#include "../lib/decode.h"
#include "../lib/MSB_LSB.h"
#include "../lib/send_message.h"


// Mutex for clients array
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

// 関数プロトタイプ
uint8_t return_str_MSB(uint16_t strlength);
uint8_t return_str_LSB(uint16_t strlength);
uint16_t combine_MSB_LSB(uint8_t msb, uint8_t lsb);
unsigned char* encode_Remining_length(int length);
unsigned int decode_remaining_length(unsigned char *encoded_bytes);
unsigned char* return_connack();
unsigned char* return_suback(MQTT_fixed_header *fh);
unsigned char* send_publish_command(MQTT_fixed_header *cfh);
void print_struct_values(MQTT_fixed_header *fh, MQTT_variable_header_protocol_name *phpn, MQTT_variable_Header_in_connect *vh, MQTT_variable_Header_in_connect *ph);
void *handle_client(void *arg);
void handle_client_connect(int socket_fd, struct sockaddr_in client_addr, char *client_id);
void handle_client_disconnect(int socket_fd);
void send_message_to_client(int socket_fd, unsigned char *message, int message_length);
void control_topic_subscriber(int socket_fd, struct sockaddr_in client_addr, char *TOPICID , uint8_t request_QoS_level , int message_id);
void unsubscribe_topic(int socket_fd , char *TOPICID);
void init_subscriber_info();

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

void init_subscriber_info() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        subscriber_info[i].socket_fd_for_subscriber = -1;
    }
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int yes = 1;
    char ret;

    init_subscriber_info();

    // ソケットの作成
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(ret != 0) {
        perror("set sock error");
        close(server_fd);
        return ret;
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

void print_binary(unsigned char byte) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (byte >> i) & 1);
    }
    printf("\n");
}

void *handle_client(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    char buffer[BUFFER_SIZE];
    int valread;
    char remaining_length_byte[4]; 
    int Packet_Length =0;
    printf("Client %d connected\n", client->id);

    while ((valread = read(client->socket_fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[valread] = '\0';
        printf("Received message from client %d: %d bytes\n", client->id, valread);

        unsigned char command_type = buffer[0] >> 4;

        printf("command type : %d\n", command_type);

        print_bits("command type", &command_type , sizeof(command_type));

        print_bits("packet contents", (unsigned char *)buffer, valread);

        char *p = buffer;
        switch (command_type) {
            case 1: { // CONNECT message
                printf("CONNECT message received from client %d\n", client->id);
                int remaining_length_byte_count = 0;
                while(1){
                    if(buffer[remaining_length_byte_count + 1] >> 7 == 1){
                        remaining_length_byte_count +=1;
                    }else{
                        break;
                    }
                }
                // printf("before\n");
                // printf("remaining_length_byte_count %d\n", remaining_length_byte_count);
                memcpy(remaining_length_byte , &buffer[1] , remaining_length_byte_count+1);
                Packet_Length = decode_remaining_length(remaining_length_byte);
                // printf("packet length : %d\n", Packet_Length);
        
                MQTT_variable_header_protocol_name *phpn = (MQTT_variable_header_protocol_name*)(p + remaining_length_byte_count + 1 +1);
                // printf("protocol_name_length_MSB : %d\n", phpn->protocol_name_length_MSB);
                // printf("protocol_name_length_LSB : %d\n", phpn->protocol_name_length_LSB);
                // printf("protocol name length  : %d\n", combine_MSB_LSB(phpn->protocol_name_length_MSB , phpn->protocol_name_length_LSB));
                // printf("protocol name  : %s\n", phpn->protocol_name);

                MQTT_variable_Header_in_connect *mphic = (MQTT_variable_Header_in_connect *)((char*)phpn + sizeof(MQTT_variable_header_protocol_name) + combine_MSB_LSB(phpn->protocol_name_length_MSB , phpn->protocol_name_length_LSB) );
                MQTT_payload_header_in_connect *phic = (MQTT_payload_header_in_connect *)((char*) mphic + sizeof(MQTT_variable_Header_in_connect));
                printf("protocol version : %d\n", mphic->protocol_version);
                char client_id[BUFFER_SIZE];
                if(mphic->something_flags >> 7 ==1){
                    strncpy(client_id, phic->clientID, BUFFER_SIZE + 1);
                    client_id[BUFFER_SIZE + 1] = '\0';
                }else{
                    strcpy(client_id , "non client ID\0");
                };
                
                pthread_mutex_lock(&clients_mutex);
                handle_client_connect(client->socket_fd, client->client_addr, client_id);
                strncpy(client->client_id, client_id, BUFFER_SIZE - 1);
                client->client_id[BUFFER_SIZE - 1] = '\0';
                pthread_mutex_unlock(&clients_mutex);

                unsigned char *return_connack_packet = return_connack();
                if (return_connack_packet == NULL) {
                    fprintf(stderr, "Error creating CONNACK packet\n");
                    close(client->socket_fd);
                    return NULL;
                }
                print_bits("connack packet", return_connack_packet , sizeof(return_connack_packet));
                send_message_to_client(client->socket_fd, return_connack_packet, sizeof(MQTT_fixed_header));
                break;
            }
    //         case 3: { // PUBLISH message
    //             printf("PUBLISH message received from client %d\n", client->id);
    //             break;
    //         }
    //         case 8: {
    //             MQTT_fixed_header cfh;
    //             printf("SUBSCRIBE message received from client %d\n", client->id);
    //             MQTT_payload_message_id_header_in_publish *pmih = (MQTT_payload_message_id_header_in_publish *)(fh + sizeof(MQTT_fixed_header) + (remaining_length_ /128)+1 );
    //             MQTT_payload_topic_id_header_in_subscribe *mptih = (MQTT_payload_topic_id_header_in_subscribe*)(fh + sizeof(MQTT_fixed_header) + sizeof(MQTT_payload_message_id_header_in_publish) + (remaining_length_/128) +1);
    //             printf("mptih->TOPICID  :  %s\n", mptih->TOPICID);
    //             control_topic_subscriber(client->socket_fd, client->client_addr, mptih->TOPICID, mptih->request_QoS_level ,combine_MSB_LSB(pmih->MESSAGE_ID_length_MSB , pmih->MESSAGE_ID_length_LSB));
    //             unsigned char *return_suback_packet=return_suback(&cfh);
    //             if (return_suback_packet == NULL) {
    //                 fprintf(stderr, "Error creating SUBNACK packet\n");
    //                 close(client->socket_fd);
    //                 return NULL;
    //             }
    //             send_message_to_client(client->socket_fd, return_suback_packet, 1024);
    //             break;
    //         }
            default:
                fprintf(stderr, "Unsupported Control Packet Type: %d\n", command_type);
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