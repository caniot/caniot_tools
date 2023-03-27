#ifndef COMM_IF_H
#define COMM_IF_H
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
typedef bool (*communication_fnc)(uint8_t *DataToSend, uint16_t data_length, uint8_t *rxDataArray, uint16_t receive_array_size);
extern void communication_if_init(void);
communication_fnc get_communication_if(void);
#endif