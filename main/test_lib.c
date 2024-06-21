#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> // Include pthread library
#include "type_h.h"
#include "../lib/MSB_LSB.h"

// uint8_t return_str_MSB(uint16_t strlength);
// uint8_t return_str_LSB(uint16_t strlength);
// uint16_t combine_MSB_LSB(uint8_t msb, uint8_t lsb);


int main(){

printf("%d\n",combine_MSB_LSB(0,3));

return 0 ;
}