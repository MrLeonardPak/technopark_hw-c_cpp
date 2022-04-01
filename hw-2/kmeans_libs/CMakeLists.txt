cmake_minimum_required(VERSION 3.0.0)
project(kmeans_lib)

set(KMEANS_LIB_LIBRARIES)
set(KMEANS_LIB_INCLUDE_DIRS)
set(KMEANS_LIB_SOURCES)

if (MULTIPROCESS)
  file(GLOB SOURCES src/kmeans.c src/kmeans_multiprocess.c)
  file(GLOB INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
  add_library(${PROJECT_NAME} STATIC ${SOURCES})
else()
  file(GLOB SOURCES src/kmeans.c src/kmeans_serial.c)
  file(GLOB INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
  add_library(${PROJECT_NAME} SHARED ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

set(KMEANS_LIB_LIBRARIES ${PROJECT_NAME} PARENT_SCOPE)
set(KMEANS_LIB_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)
set(KMEANS_LIB_SOURCES ${SOURCES} PARENT_SCOPE)