cmake_minimum_required(VERSION 3.16)

# Set custom components directory to locate local BSP component
set(EXTRA_COMPONENT_DIRS "../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(bsp_sensors_example)
