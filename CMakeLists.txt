cmake_minimum_required(VERSION 3.5)

if (NOT DEFINED PROJECT_NAME)
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)

  list(APPEND EXTRA_COMPONENT_DIRS components/max31855)

  project(max31855-example)
endif (NOT DEFINED PROJECT_NAME)
