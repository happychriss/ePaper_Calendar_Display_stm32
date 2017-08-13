# specify the cross compiler

cmake_minimum_required(VERSION 3.5.0)
add_definitions(-DSTM32F10X_XL)
include_directories(include)
file(GLOB_RECURSE STM_SOURCES  *.c)
add_library(stm32f10x STATIC ${STM_SOURCES})