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
    if (i < 0 || i > 7) {
        perror("Error: bit index out of range\n");
        return;
    }
    *ptr |= (1 << i-1);
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


// unsigned char* return_suback(MQTT_fixed_header *fh) {
//     printf("create suback message\n");
//     fh->Control_Packet_type = 0x09; // SUBACK
//     unsigned char *FX_LENGTH = encode_Remining_length(FIXED_SIZE + sizeof(MQTT_payload_message_id_header_in_publish));
//     if (FX_LENGTH == NULL) {
//         return NULL;
//     }
//     memcpy(fh->Remaining_Length, FX_LENGTH, FIXED_SIZE);
//     free(FX_LENGTH);

//     MQTT_payload_message_id_header_in_publish *pmih  = (MQTT_payload_message_id_header_in_publish *)(fh + sizeof(MQTT_fixed_header));
//     pmih->MESSAGE_ID_length_MSB = return_str_MSB(MESSAGE_ID_SIZE);
//     pmih->MESSAGE_ID_length_LSB = return_str_LSB(MESSAGE_ID_SIZE);
//     return (unsigned char *)fh;
// }

unsigned char* return_connack() {
    printf("create connack message\n");
    unsigned char *connack_packet;
    connack_packet = (unsigned char *)malloc(sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_connack));
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
