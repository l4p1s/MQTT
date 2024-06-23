#ifndef _CALC_H_
#define _CALC_H_
 
unsigned char* return_suback(unsigned char* pmih , unsigned char* mptih , int topic_count);
unsigned char* send_publish_command(MQTT_fixed_header *cfh);
unsigned char* return_connack();
 
#endif // _CALC_H_