#ifndef _CALC_H_
#define _CALC_H_
 
unsigned char* return_suback(MQTT_fixed_header *fh);
unsigned char* send_publish_command(MQTT_fixed_header *cfh);
unsigned char* return_connack(MQTT_fixed_header *fh);
 
#endif // _CALC_H_