#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "encode.h"
#include "build_command.h"
#include "MSB_LSB.h"
#include "lib_struct.h"





void setBit(unsigned char *ptr, int i) {
    if (i < 1 || i > 8) {
        perror("Error: bit index out of range\n");
        return;
    }
    *ptr |= (1 << (i - 1));
}


unsigned char* return_suback(char* p, int topic_count, int remaining_length_byte_count) {
    printf("create suback message\n");
    unsigned char *suback_packet;
    suback_packet = (unsigned char *)malloc((sizeof(MQTT_fixed_header) + 1 + sizeof(MQTT_variable_header_in_suback) + topic_count));
    setBit(&suback_packet[0], 8);
    setBit(&suback_packet[0], 5);
    printf("topic_count  : %d\n", topic_count);
    unsigned char* encoded_bytes = encode_Remining_length(sizeof(MQTT_variable_header_in_suback) + topic_count);
    MQTT_payload_message_id_header *pmih = (MQTT_payload_message_id_header *)(p + remaining_length_byte_count + 1);
    
    // Copying single bytes using assignment instead of strncpy
    suback_packet[1] = encoded_bytes[0];
    suback_packet[2] = pmih->MESSAGE_ID_length_MSB;
    suback_packet[3] = pmih->MESSAGE_ID_length_LSB;
    
    MQTT_payload_topic_id_header_in_subscribe *mptih = (MQTT_payload_topic_id_header_in_subscribe*)((char*)pmih + sizeof(MQTT_payload_message_id_header));
    for(int i = 1 ; i < topic_count + 1 ; i++){
        int topic_length = combine_MSB_LSB(mptih->TOPIC_ID_length_MSB, mptih->TOPIC_ID_length_LSB);        
        char *request_QoS_level = ((char*)mptih + sizeof(MQTT_payload_topic_id_header_in_subscribe) + topic_length - 1);
        suback_packet[3 + i] = *request_QoS_level; // 修正箇所
        mptih = (MQTT_payload_topic_id_header_in_subscribe *)((uint8_t *)mptih + sizeof(MQTT_payload_topic_id_header_in_subscribe) + topic_length);
    }
    
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
