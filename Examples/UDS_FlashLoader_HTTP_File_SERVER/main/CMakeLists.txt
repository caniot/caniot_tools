set(COMPONENT_SRCS  "main.c" 
                    "actions_task.c"                  
                    "UDS_FLASH/crc.c"
                    "UDS_FLASH/uds_service_fnc.c"
                    "UDS_FLASH/uds_lvgl_appl.c"
                    "UDS_FLASH/main_flash.c"
                    "UDS_FLASH/communication_if.c"
                    "main_fs/file_server.c"
                     EMBED_FILES "main_fs/favicon.ico" "main_fs/upload_script.html")
                     
set(COMPONENT_ADD_INCLUDEDIRS "."  "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/includes" "main_fs" "UDS_FLASH/")
set(COMPONENT_REQUIRES)
register_component()
if("esp32s3" STREQUAL "${IDF_TARGET}")
    if(CONFIG_LV_TFT_DISPLAY_CONTROLLER_ILI9488)
        add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib_s31.a" REQUIRES  lvgl tinyusb)
    else()
        add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib_s3.a" REQUIRES  lvgl tinyusb)
    endif()
elseif("esp32" STREQUAL "${IDF_TARGET}")
add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib_s1.a" REQUIRES  lvgl)
endif()
target_link_libraries(${COMPONENT_LIB} libprebuilt)

