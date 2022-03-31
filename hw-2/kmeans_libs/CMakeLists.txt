cmake_minimum_required(VERSION 3.0.0)
project(kmeans_lib)

option(MULTI "Do you want to run multiplex variant?" OFF)

set(KMEANS_LIB_LIBRARIES)
set(KMEANS_LIB_INCLUDE_DIRS)

if (MULTI)
  file(GLOB SOURCES src/kmeans.c src/kmeans_multiplex.c)
  file(GLOB INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
  add_library(${PROJECT_NAME} STATIC ${SOURCES})
else()
  file(GLOB SOURCES src/kmeans.c src/kmeans_serial.c)
  file(GLOB INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
  add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

set(KMEANS_LIB_LIBRARIES ${PROJECT_NAME} PARENT_SCOPE)
set(KMEANS_LIB_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)