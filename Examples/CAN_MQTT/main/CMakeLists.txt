set(COMPONENT_SRCS  "main.c" 
                    "actions_task.c" 
                    "CAN/can_lvgl_appl.c" 
                    "CAN/canApplication.c"
                   )
set(COMPONENT_ADD_INCLUDEDIRS "."  "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/includes" "CAN/")
set(COMPONENT_REQUIRES)
register_component()
add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib.a" REQUIRES  lvgl)

target_link_libraries(${COMPONENT_LIB} libprebuilt)

