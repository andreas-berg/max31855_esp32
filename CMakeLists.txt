#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

list(APPEND EXTRA_COMPONENT_DIRS 
  components/max31855/components/max31855
)

project(max31855-example)
