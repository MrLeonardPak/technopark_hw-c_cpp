cmake_minimum_required(VERSION 3.14.0)
project(test_schedule)

file(GLOB SOURCES *.cc)
file(GLOB INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_GMOCK "Do you want to build gmonk?" OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        main
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SCHEDULE_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SCHEDULE_LIB_LIBRARIES} gtest_main)