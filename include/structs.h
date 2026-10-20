#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

typedef struct {
    uint32_t addr;
    uint32_t data;
    uint8_t  w; // 1 if a write request and 0 if a read request
    uint8_t  user;
    uint8_t  wide; // 4 if T and 1 if F
} Request;



typedef struct{
   uint32_t cycles;
   uint32_t errors;
} Result;

#endif