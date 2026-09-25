cmake_minimum_required(VERSION 3.16)

# Default to esp32s3 target if not explicitly set
if(NOT IDF_TARGET)
    set(IDF_TARGET "esp32s3")
endif()

list(APPEND EXTRA_COMPONENT_DIRS "../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(unified_bsp_demo)
