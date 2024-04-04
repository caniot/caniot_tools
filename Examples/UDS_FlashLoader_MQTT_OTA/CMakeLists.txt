# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(PROJECT_VER  "01.00")
set(MQTT_VARIANT_PUBLISCHER  "0")
set(compile_options "-DCONFIG_TINYUSB_TASK_STACK_SIZE=4096" "-DCONFIG_TINYUSB_TASK_PRIORITY=5" )
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
if("1" STREQUAL "${MQTT_VARIANT_PUBLISCHER}" )
project(UDS_FlashLoader_MQTT_OTA_PUB)
else()
project(UDS_FlashLoader_MQTT_OTA_SUB)
endif()