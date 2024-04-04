#ifndef TYPES_H
#define TYPES_H
#include "cJSON.h"
#define MAX_FILE_TOFLASH 10
typedef struct flash
{
    char *service_name;
    bool (*service_fnc)(const cJSON *const mainArray);
} flashLoader_obj_t;



extern appl_message_t appl_messagefile[MAX_FILE_TOFLASH];
extern void uds_service_init();
extern const flashLoader_obj_t *uds_service_getData(int *numberOfServices);
#endif