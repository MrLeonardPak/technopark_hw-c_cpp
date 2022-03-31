cmake_minimum_required(VERSION 3.0.0)
project(hw-2)

option(TESTS "Do you want to build test?" OFF)
option(WITH_STATIC_CHECKS "Do you want to use static checks?" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if (TESTS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage -lgcov" )
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Wpedantic -Wextra -Werror -fprofile-arcs -ftest-coverage -fPIC -O0")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Wpedantic -Wextra -Werror -fprofile-arcs -ftest-coverage -fPIC -O0")
else()
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Wpedantic -Wextra -Werror -O0")
  set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS} "-Wall -Wpedantic -Wextra -Werror -O0")
endif()

add_subdirectory(kmeans_libs)

message("KMEANS_LIB_LIBRARIES = ${KMEANS_LIB_LIBRARIES}")
message("KMEANS_LIB_INCLUDE_DIRS = ${KMEANS_LIB_INCLUDE_DIRS}")

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PUBLIC ${KMEANS_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KMEANS_LIB_LIBRARIES} m)

if (TESTS)
  add_subdirectory(tests)
endif()

if(WITH_STATIC_CHECKS)
  # Add clang-tidy with clang-analyzer (scan-build)
  set(CMAKE_C_CLANG_TIDY 
    clang-tidy-12;
    -header-filter=.;
    -warnings-as-errors=*;
    -extra-arg=-std=c99;
    -checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus*)

  # Add clang-format-12 to make target
  add_custom_target(
          clangformat
          COMMAND clang-format-12
          -style=Chromium
          -n
          --Werror
          ${CMAKE_CURRENT_SOURCE_DIR}/*.c
          ${KMEANS_LIB_INCLUDE_DIRS}/*.h
          ${KMEANS_LIB_SOURCES}
          ${TESTS_SCHEDULE_INCLUDE_DIRS}/*.cc
  )
endif()

