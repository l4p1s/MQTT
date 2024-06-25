#ifndef _CALC_H_
#define _CALC_H_
 
unsigned char* return_suback(char *p , int topic_count , int remaining_length_byte_count);
// unsigned char* send_publish_command(MQTT_fixed_header *cfh);
unsigned char* return_connack();
 


#endif // _CALC_H_