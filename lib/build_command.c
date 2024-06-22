#include <stdio.h>
#include <stdlib.h>
#include <string.h>



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
