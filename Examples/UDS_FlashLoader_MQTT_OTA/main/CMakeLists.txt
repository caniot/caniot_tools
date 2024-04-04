set(SOURCES 
            "main.c" 
            "actions_task.c"
            "lvgl_appl.c")

if("1" STREQUAL "${MQTT_VARIANT_PUBLISCHER}" )
                            list(APPEND SOURCES "MQTT_HIVEMQ/mqtt_HiveMQ_publisher.c")
                            add_compile_definitions("MQTT_VARIANT_PUBLISCHER")
else()
                            list(APPEND SOURCES "MQTT_HIVEMQ/mqtt_HiveMQ_subscriber.c"            
                            "UDS_FLASH/crc.c"
                            "UDS_FLASH/uds_service_fnc.c"
                            "UDS_FLASH/uds_lvgl_appl.c"
                            "UDS_FLASH/main_flash.c"
                            "UDS_FLASH/communication_if.c")
endif()

idf_component_register(SRCS "${SOURCES}"
                       INCLUDE_DIRS "."  "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/includes" "UDS_FLASH/" "MQTT_HIVEMQ/"
                       EMBED_TXTFILES ${CMAKE_CURRENT_SOURCE_DIR}/server_certs/ca_cert_mqtt.pem)

if("esp32s3" STREQUAL "${IDF_TARGET}")
    if(CONFIG_LV_TFT_DISPLAY_CONTROLLER_ILI9488)
        add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib_s31.a" REQUIRES  lvgl)
    else()
        add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib_s3.a" REQUIRES  lvgl)
    endif()
elseif("esp32" STREQUAL "${IDF_TARGET}")
add_prebuilt_library(libprebuilt "${CMAKE_CURRENT_SOURCE_DIR}/.caniotBoxLib/caniotBoxLib_s1.a" REQUIRES  lvgl)
endif()
target_link_libraries(${COMPONENT_LIB} libprebuilt)

