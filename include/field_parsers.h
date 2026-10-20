#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int parseType(char*type, uint8_t* w );

int parseAdress(char* adress, uint32_t* addr);

int parseData(char* data, uint32_t* d, const uint8_t w, const uint8_t width);

int parseUser(char* user,uint8_t* usr);

int parseWidth(char* wide, uint8_t* width);