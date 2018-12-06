project(stm32_display C ASM)
cmake_minimum_required(VERSION 3.5.0)
add_definitions(-DSTM32F10X_XL)


add_subdirectory(stm32f10x)

include_directories(stm32_application)
include_directories(stm32f10x/include)
include_directories(stm32_application/images)
include_directories(stm32f10x/include)
include_directories(stm32f10x/include/stm32f10x)


file(GLOB_RECURSE USER_SOURCES  stm32_application/*.c)
file(GLOB_RECURSE USER_HEADERS  stm32_application/*.h stm32_application/images/*.h)

add_executable(${PROJECT_NAME}.elf ${STM_HEADERS} ${USER_SOURCES} ${USER_HEADERS} ${LINKER_SCRIPT} stm32_application/usart.c stm32_application/usart.h stm32_application/init_stm32.c stm32_application/init_stm32.h stm32_application/support_functions.c stm32_application/support_functions.h stm32_application/painter.c stm32_application/painter.h stm32_application/fonts/fontem.h)

target_link_libraries(${PROJECT_NAME}.elf stm32f10x)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.map")
set(HEX_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.bin)

# The file gdb_server_init_load_elf contains commands for the arm-none-eabi-gdb to transfer the program to target
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMAND arm-none-eabi-gdb --command=/home/development/Projects/eInkCalendar/stm32_display/gdb_server_init_load_elf
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")