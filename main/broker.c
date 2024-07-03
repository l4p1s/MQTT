#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
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
// pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t subscribers_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;



void control_topic_subscriber(int socket_fd, struct sockaddr_in client_addr, char *TOPICID , uint8_t request_QoS_level , int message_id ,TOPIC_INFO *subscriber_info){
    for(int i=0 ; i < MAX_CLIENTS ; i++){
        if(subscriber_info[i].socket_fd_for_subscriber == -1){
            subscriber_info[i].socket_fd_for_subscriber = socket_fd;
            subscriber_info[i].client_addr = client_addr;
            subscriber_info[i].cur_message_id = message_id;
            subscriber_info[i].request_QoS_level = request_QoS_level;
            strcpy(subscriber_info[i].client_topic , TOPICID);
            printf("subscribe topic %s \n", subscriber_info[i].client_topic);
            printf("subscribe topic QoS %d \n", subscriber_info[i].request_QoS_level);
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
            memcpy(clients[i].client_id, client_id, BUFFER_SIZE - 1);
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

void init_subscriber_info(TOPIC_INFO * subscriber_info) {
    for (int i = 0; i < 20; ++i) {
        subscriber_info[i].socket_fd_for_subscriber = -1;
    }
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int yes = 1;
    char ret;
    pid_t pid;
    TOPIC_INFO *mmmap_ptr;

    
    mmmap_ptr = (TOPIC_INFO *)mmap(NULL, sizeof(subscriber_info[MAX_CLIENTS]), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mmmap_ptr == MAP_FAILED) {
        perror("mmap failed\n");
        exit(1);
    }
    init_subscriber_info(mmmap_ptr);
    
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
    if (listen(server_fd, 100) < 0) {
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
        // pthread_mutex_lock(&clients_mutex);
        clients[client_count].id = client_count;
        clients[client_count].socket_fd = new_socket;
        clients[client_count].client_addr = address;
        if((pid = fork()) == -1) {
            perror("fork failed\n");
            return 1;
        }else if(pid == 0){
            handle_client(&clients[client_count] , mmmap_ptr);
            exit(0);
        }else if(pid > 0){
            client_count++;
        }
        // pthread_mutex_unlock(&clients_mutex);
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


void forward_publish_message_to_subscribers(char *topic_, unsigned char *message, int message_length , TOPIC_INFO *subscriber_info) {
    printf("Forwarding function\n");
    printf("Received topic: %s\n", topic_);
    printf("Received topic length: %zu\n", strlen(topic_));
    printf("mssage length : %d\n", message_length);
    // pthread_mutex_lock(&subscribers_mutex);
    print_bits("forward contents", message, message_length);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (subscriber_info[i].socket_fd_for_subscriber != -1) {
            if (strcmp(topic_, subscriber_info[i].client_topic) == 0) {
                printf("Forwarding PUBLISH message to subscriber on topic %s\n", topic_);
                send_message_to_client(subscriber_info[i].socket_fd_for_subscriber, message, message_length );
            }
        }
    }
    // pthread_mutex_unlock(&subscribers_mutex);
}


void *handle_client(void *arg1 , void *arg2) {
    ClientInfo *client = (ClientInfo *)arg1;
    TOPIC_INFO *subscriber_info = (TOPIC_INFO *)arg2;
    char *buffer;
    buffer = (char *)malloc(BUFFER_SIZE);
    int valread;
    char remaining_length_byte[4]; 
    int Packet_Length = 0;
    printf("Client %d connected\n", client->id);

    int total_read_packet_size = 0 ;

    while ((valread = read(client->socket_fd, buffer, BUFFER_SIZE)) > 0) {

        total_read_packet_size += valread;
        printf("total packet read  : %d\n", total_read_packet_size);

        int remaining_length_byte_count = 1;
        while(1){
            if(buffer[remaining_length_byte_count + 1] >> 7 == 1){
                remaining_length_byte_count +=1;
            }else{
                break;
            }
        }

        memcpy(remaining_length_byte , &buffer[1] , remaining_length_byte_count+1);
        Packet_Length = decode_remaining_length(remaining_length_byte);

        // whileとforどっちがいいんだろう
        while(total_read_packet_size == Packet_Length + sizeof(MQTT_fixed_header)){
            valread = read(client->socket_fd, buffer + total_read_packet_size, BUFFER_SIZE - total_read_packet_size);
            total_read_packet_size += valread;
            printf("total read packet length  : %d\n", total_read_packet_size);
        }

        buffer[valread] = '\0';
        printf("Received message from client %d: %d bytes\n", client->id, valread);
        printf("packet length  : %d\n", Packet_Length);
        
        unsigned char command_type = buffer[0] >> 4;
        COMMAND_TYPE ctype = (unsigned int)command_type;

        print_bits("packet contents", (unsigned char *)buffer, valread);
        // printf("recieved fd  : %d\n", client->socket_fd);

        char *p = buffer;
        // int remaining_length_byte_count = 1;
        // while(1){
        //             if(buffer[remaining_length_byte_count + 1] >> 7 == 1){
        //                 remaining_length_byte_count +=1;
        //             }else{
        //                 break;
        //             }
        //         }
        //         memcpy(remaining_length_byte , &buffer[1] , remaining_length_byte_count+1);
        //         Packet_Length = decode_remaining_length(remaining_length_byte);

        switch (ctype) {
            case CONNECT: { // CONNECT message
                printf("CONNECT message received from client %d\n", client->id);
        
                MQTT_variable_header_protocol_name *phpn = (MQTT_variable_header_protocol_name*)(p + remaining_length_byte_count + 1);

                MQTT_variable_Header_in_connect *mphic = (MQTT_variable_Header_in_connect *)((char*)phpn + sizeof(MQTT_variable_header_protocol_name) + combine_MSB_LSB(phpn->protocol_name_length_MSB , phpn->protocol_name_length_LSB) );
                MQTT_payload_header_in_connect *phic = (MQTT_payload_header_in_connect *)((char*) mphic + sizeof(MQTT_variable_Header_in_connect));

                char client_id[BUFFER_SIZE];
                if(mphic->something_flags >> 7 ==1){
                    memcpy(client_id, phic->clientID, BUFFER_SIZE + 1);
                    client_id[BUFFER_SIZE + 1] = '\0';
                }else{
                    strcpy(client_id , "non client ID\0");
                };
                
                // pthread_mutex_lock(&clients_mutex);
                handle_client_connect(client->socket_fd, client->client_addr, client_id);
                memcpy(client->client_id, client_id, BUFFER_SIZE - 1);
                client->client_id[BUFFER_SIZE - 1] = '\0';
                // pthread_mutex_unlock(&clients_mutex);

                unsigned char *return_connack_packet = return_connack();
                if (return_connack_packet == NULL) {
                    fprintf(stderr, "Error creating CONNACK packet\n");
                    close(client->socket_fd);
                    return NULL;
                }
                print_bits("conack packet contents", return_connack_packet , sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_connack));
                // printf("connack socket_fd  : %d\n", client->socket_fd);
                send_message_to_client(client->socket_fd, return_connack_packet, sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_connack));
                break;
            }
            case PUBLISH: { // PUBLISH message
                printf("PUBLISH message received from client %d\n", client->id);
                MQTT_variable_topic_id_header_in_publish *vtihip = (MQTT_variable_topic_id_header_in_publish *)(p+remaining_length_byte_count +1);
                int topic_length = combine_MSB_LSB(vtihip->TOPIC_ID_length_MSB , vtihip->TOPIC_ID_length_LSB);
                printf("topic length : %d\n", topic_length);
                char *cur_topic_id = (char *)malloc(topic_length);
                if (cur_topic_id == NULL) {
                    perror("Memory allocation failed\n");
                }
                memcpy(cur_topic_id, vtihip->TOPICID, topic_length);

                printf("topic id : %s\n", cur_topic_id);
                printf("packet length  : %d\n" ,valread);
                // delite Disconnect Req
                forward_publish_message_to_subscribers(cur_topic_id, p , valread - 2 , subscriber_info);
                free(cur_topic_id);
                break;
            }
            case SUBSCRIBE : {
                printf("SUBSCRIBE message received from client %d\n", client->id);

                MQTT_payload_message_id_header *pmih = (MQTT_payload_message_id_header *)(p + remaining_length_byte_count + 1 );
                printf("message length  : %d\n", combine_MSB_LSB(pmih->MESSAGE_ID_length_MSB , pmih->MESSAGE_ID_length_LSB));
                MQTT_payload_topic_id_header_in_subscribe *mptih = (MQTT_payload_topic_id_header_in_subscribe*)((char*)pmih + sizeof(MQTT_payload_message_id_header));
                int topic_count = 0;
                int topic_flag = sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_payload_message_id_header);
                while (1) {
                    topic_count++;
                    printf("topic_count  : %d\n", topic_count);
                    int topic_length = combine_MSB_LSB(mptih->TOPIC_ID_length_MSB, mptih->TOPIC_ID_length_LSB);
                    char *request_QoS_level = ((char*)mptih + sizeof(MQTT_payload_topic_id_header_in_subscribe) + topic_length - 1);
                    control_topic_subscriber(client->socket_fd, client->client_addr, mptih->TOPICID, (uint8_t)request_QoS_level[0], topic_length , subscriber_info);
                    topic_flag += sizeof(MQTT_payload_topic_id_header_in_subscribe) + combine_MSB_LSB(mptih->TOPIC_ID_length_MSB, mptih->TOPIC_ID_length_LSB);
                    if(topic_flag < valread){
                        mptih = (MQTT_payload_topic_id_header_in_subscribe *)((uint8_t *)mptih + sizeof(MQTT_payload_topic_id_header_in_subscribe) + topic_length);
                    }else{
                        break;
                    }
                }
                unsigned char *return_suback_packet = return_suback(p , topic_count , remaining_length_byte_count);
                if (return_suback_packet == NULL) {
                    fprintf(stderr, "Error creating SUBNACK packet\n");
                    close(client->socket_fd);
                    return NULL;
                }
                print_bits("suback packet" , return_suback_packet , sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_suback) + topic_count);
                printf("end topic\n");
                send_message_to_client(client->socket_fd, return_suback_packet, (sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_suback) + topic_count));
                break;
            }
            case DISCONNECT : {
                printf("Client %d disconnected\n", client->id);
                // pthread_mutex_lock(&clients_mutex);
                clients[client->id].socket_fd = -1;
                // pthread_mutex_unlock(&clients_mutex);
                close(client->socket_fd);
                break;
            }
            case PING : {
                printf("recieve ping\n");
                unsigned char *return_pingresp_packet = return_pingresp();
                send_message_to_client(client->socket_fd, return_pingresp_packet, sizeof(MQTT_fixed_header));
                free(return_pingresp_packet);
                break;
            }
            default:
                fprintf(stderr, "Unsupported Control Packet Type: %d\n", ctype);
                break;
        }
    }
    close(client->socket_fd);
    free(buffer);
    return NULL;
}