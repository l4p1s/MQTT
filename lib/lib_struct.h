#ifndef _STRUCT_H_
#define _STRUCT_H_
 

typedef struct {
    // shiftして1つの変数にしたがいいと思う
    uint8_t Control_Packet_type : 4;
    uint8_t Flags : 4;
    unsigned char Remaining_Length[0];
}MQTT_fixed_header;

typedef struct{
    uint8_t MESSAGE_ID_length_MSB;
    uint8_t MESSAGE_ID_length_LSB;
}MQTT_payload_message_id_header;

typedef struct{
    unsigned char no_flags[1];
    unsigned char return_code[1];
}MQTT_variable_header_in_connack;

typedef struct{
    uint8_t return_MESSAGE_ID_length_MSB;
    uint8_t return_MESSAGE_ID_length_LSB;
}MQTT_variable_header_in_suback;

typedef struct{
    uint8_t TOPIC_ID_length_MSB;
    uint8_t TOPIC_ID_length_LSB;
    char TOPICID[0];
    uint8_t request_QoS_level;
}MQTT_payload_topic_id_header_in_subscribe;


#endif