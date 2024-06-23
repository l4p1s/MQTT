#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "encode.h"



void setBit(unsigned char *ptr, int i) {
    if (i < 1 || i > 8) {
        perror("Error: bit index out of range\n");
        return;
    }
    *ptr |= (1 << (i - 1));
}


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

typedef struct{
    uint8_t return_MESSAGE_ID_length_MSB;
    uint8_t return_MESSAGE_ID_length_LSB;
}MQTT_variable_header_in_subnack;

typedef struct{
    uint8_t MESSAGE_ID_length_MSB;
    uint8_t MESSAGE_ID_length_LSB;
}MQTT_payload_message_id_header;

typedef struct{
    uint8_t TOPIC_ID_length_MSB;
    uint8_t TOPIC_ID_length_LSB;
    char TOPICID[0];
    uint8_t request_QoS_level;
}MQTT_payload_topic_id_header_in_subscribe;

unsigned char* return_suback(unsigned char* pmih , unsigned char* mptih ,int topic_count){
    printf("create suback message\n");
    unsigned char *suback_packet;
    suback_packet = (unsigned char *)malloc((sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_subnack) + topic_count));
    setBit(&suback_packet[0] , 8);
    setBit(&suback_packet[0] , 5);
    unsigned char* encoded_bytes = encode_Remining_length(sizeof(MQTT_variable_header_in_subnack) + topic_count);
    strncpy(&suback_packet[1] , encoded_bytes , 1);
    strncpy(&suback_packet[2] , pmih.MESSAGE_ID_length_MSB , 1);
    strncpy(&suback_packet[3] , pmih.MESSAGE_ID_length_LSB , 1);
    
    return (unsigned char *)suback_packet;
}

unsigned char* return_connack() {
    printf("create connack message\n");
    unsigned char *connack_packet;
    connack_packet = (unsigned char *)malloc((sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_connack)));
    // printf("size of fixed header  : %ld\n", sizeof(MQTT_fixed_header));
    // printf("size of MQTT_variable_header_in_connack  : %ld\n", sizeof(MQTT_variable_header_in_connack));
    // printf("size of connack packet  : %ld\n", sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_connack));
    // printf("size of connack packet  : %ld\n", sizeof(connack_packet));
    // メッセージタイプ2
    setBit(&connack_packet[0], 6);
    // remining length(固定長なので、直接)
    setBit(&connack_packet[1], 2);
    //接続許可のフラグ
    MQTT_variable_header_in_connack *mvhic = (MQTT_variable_header_in_connack*)((unsigned char*)connack_packet + sizeof(MQTT_fixed_header));
    memset(mvhic->return_code , 0 , sizeof(mvhic->return_code));

    return connack_packet;
}
// unsigned char* send_publish_command(MQTT_fixed_header *cfh) {
//     cfh->Control_Packet_type = 0x03;
//     cfh->Flags = 0x02; 
//     unsigned char *FX_LENGTH = encode_Remining_length(FIXED_SIZE);
//     if (FX_LENGTH == NULL) {
//         return NULL;
//     }
//     memcpy(cfh->Remaining_Length, FX_LENGTH, FIXED_SIZE);
//     free(FX_LENGTH);
//     int Remaining_Length_ = sizeof(MQTT_payload_topic_id_header_in_publish) + sizeof(MQTT_payload_message_id_header_in_publish) +  strlen(TOPIC_ID) + MESSAGE_ID_SIZE + FIXED_SIZE;
//     MQTT_payload_topic_id_header_in_publish *ptih = (MQTT_payload_topic_id_header_in_publish *)(cfh + sizeof(MQTT_fixed_header) + (Remaining_Length_ / 128) + 1);

//     ptih->TOPIC_ID_length_MSB = return_str_MSB(strlen(TOPIC_ID));
//     ptih->TOPIC_ID_length_LSB = return_str_LSB(strlen(TOPIC_ID));
//     strcpy(ptih->TOPICID, TOPIC_ID);
    
//     MQTT_payload_message_id_header_in_publish *pmih = (MQTT_payload_message_id_header_in_publish *)(ptih + sizeof(MQTT_payload_topic_id_header_in_publish)) + strlen(TOPIC_ID);

//     pmih->MESSAGE_ID_length_MSB = return_str_MSB(MESSAGE_ID_SIZE);
//     pmih->MESSAGE_ID_length_LSB = return_str_LSB(MESSAGE_ID_SIZE);

//     MQTT_payload_message *mpm = (MQTT_payload_message *)(pmih + sizeof(MQTT_payload_message_id_header_in_publish));
//     memcpy(mpm->message, EXAMPLE_MASSAGE , sizeof(EXAMPLE_MASSAGE));

//     return (unsigned char *)cfh;
// }
