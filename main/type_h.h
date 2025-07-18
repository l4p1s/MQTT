#ifndef STRUCT_TYPE_H
#define STRUCT_TYPE_H

#define Hide "hide"
#define None "none"
#define PORT 8000
#define PROTOCOL_NAME "MQTT"
#define MASK  0x000000ff
#define MAX_1BYTE_LENGTH 255
#define CLIENTID "client1"


#define MAX_CLIENTS 100
#define CLIENT_BUFFER_SIZE 24
#define BUFFER_SIZE 1024

// クライアント情報の構造体
typedef struct {
    int id;
    int socket_fd;
    struct sockaddr_in client_addr;
    char client_id[CLIENT_BUFFER_SIZE];
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];

typedef struct{
    int id;
    int socket_fd_for_subscriber;
    uint8_t request_QoS_level;
    int cur_message_id;
    struct sockaddr_in client_addr;
    char client_topic[CLIENT_BUFFER_SIZE];
}TOPIC_INFO;

TOPIC_INFO subscriber_info[MAX_CLIENTS];

typedef struct {
    // shiftして1つの変数にしたがいいと思う
    uint8_t Control_Packet_type : 4;
    uint8_t Flags : 4;
    unsigned char Remaining_Length[0];
}MQTT_fixed_header;

typedef struct{
    unsigned char no_flags[1];
    unsigned char return_code[1];
}MQTT_variable_header_in_connack;

typedef struct {
    // 2byteのshort intで定義して、シフト＆0x000000FFでマスク、　MSBとLSBに入れる。　
    uint8_t protocol_name_length_MSB ; // 1byte
    uint8_t protocol_name_length_LSB ; // 1byte
    char protocol_name[0];  // 4byte
}MQTT_variable_header_protocol_name;

typedef struct{
    uint8_t protocol_version; // 1byte
    uint8_t something_flags ; // 1byte
    uint8_t keep_alive_timer_MSB ; // 1byte
    uint8_t keep_alive_timer_LSB ; // 1byte
 
}MQTT_variable_Header_in_connect;

typedef struct{
    uint8_t clientID_length_MSB;
    uint8_t clientID_length_LSB;
    char clientID[0] ;

}MQTT_payload_header_in_connect;


typedef struct{
    uint8_t TOPIC_ID_length_MSB;
    uint8_t TOPIC_ID_length_LSB;
    char TOPICID[0];
    uint8_t MESSAGE_ID_length_MSB;
    uint8_t MESSAGE_ID_length_LSB;
}MQTT_variable_topic_id_header_in_publish;

typedef struct{
    uint8_t TOPIC_ID_length_MSB;
    uint8_t TOPIC_ID_length_LSB;
    char TOPICID[0];
    uint8_t request_QoS_level;
}MQTT_payload_topic_id_header_in_subscribe;


typedef struct{
    uint8_t MESSAGE_ID_length_MSB;
    uint8_t MESSAGE_ID_length_LSB;
}MQTT_payload_message_id_header;

typedef struct{
    uint8_t return_MESSAGE_ID_length_MSB;
    uint8_t return_MESSAGE_ID_length_LSB;
}MQTT_variable_header_in_suback;

typedef struct{
    char message[0];
}MQTT_payload_message;

#endif /* STRUCT_TYPE_H */