#ifndef LVGL_IF_H_
#define LVGL_IF_H_

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif
/*main table view  used as parent to create new object*/
extern lv_obj_t *main_table;
/*charts table view used as parent to create new object*/
extern lv_obj_t *charts_table;
/*utility table view used as parent to create new object*/
extern lv_obj_t *utility_table;

 
/***************************************************************************
* @name      lvgl_if_take 
* @brief     Occupy  semaphore to handle concurrent call to lvgl stuff.
*            If you wish to call *any* lvgl function from other threads/tasks
*            you should call this function first 
*            except function assigned to lv_task_create
*
* @attention 1. This API should be called at first, before all functions
* @param     void
*
* @return    esp_err_t
*****************************************************************************/
extern esp_err_t lvgl_if_take(void);
/***************************************************************************
* @name      lvgl_if_give 
* @brief     release  semaphore to handle concurrent call to lvgl stuff.
*
* @attention 1. This API should be called at first, before all functions
* @param     void
*
* @return    esp_err_t
*****************************************************************************/
extern esp_err_t lvgl_if_give(void);
#endif