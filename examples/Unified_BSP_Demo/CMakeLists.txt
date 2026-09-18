cmake_minimum_required(VERSION 3.16)

# Add BSP component search path
list(APPEND EXTRA_COMPONENT_DIRS "../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(unified_bsp_demo)
