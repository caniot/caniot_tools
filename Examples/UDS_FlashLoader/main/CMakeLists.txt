set(COMPONENT_SRCS  "main.c" 
                    "actions_task.c" 
                    "UDS_FLASH/crc.c"
                    "UDS_FLASH/uds_service_fnc.c"
                    "UDS_FLASH/uds_lvgl_appl.c"
                    "UDS_FLASH/main_flash.c"
                    "UDS_FLASH/communication_if.c"
                   )
set(COMPONENT_ADD_INCLUDEDIRS "."  "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/includes" "UDS_FLASH/")
set(COMPONENT_REQUIRES)
register_component()
add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib.a" REQUIRES  lvgl)

target_link_libraries(${COMPONENT_LIB} libprebuilt)

