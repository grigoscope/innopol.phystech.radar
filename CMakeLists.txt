cmake_minimum_required(VERSION 3.12)

project(OPTIMIZED_ANT LANGUAGES C) 

file(GLOB PROJECT_SOURCE_FILES CONFIGURE_DEPENDS src/*.c)
include_directories(include/)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})