cmake_minimum_required(VERSION 3.16)

# Locate local BSP component
set(EXTRA_COMPONENT_DIRS "../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(humid1_os_boilerplate)
