cmake_minimum_required(VERSION 3.0.0)
project(hw-1)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Wpedantic -Wextra -Werror")

add_subdirectory(schedule)

message("SCHEDULE_LIB_INCLUDE_DIRS = ${SCHEDULE_LIB_INCLUDE_DIRS}")
message("SCHEDULE_LIB_LIBRARIES = ${SCHEDULE_LIB_LIBRARIES}")

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PUBLIC ${SCHEDULE_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SCHEDULE_LIB_LIBRARIES})
