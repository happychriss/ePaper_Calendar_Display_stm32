project(stm32_display C ASM)
cmake_minimum_required(VERSION 3.5.0)
add_definitions(-DSTM32F10X_XL)

add_subdirectory(stm32f10x)

include_directories(stm32_application)
include_directories(stm32f10x/include)
include_directories(stm32_application/images)
include_directories(stm32f10x/include)
include_directories(stm32f10x/include/stm32f10x)
include_directories(FatFS)


file(GLOB_RECURSE USER_SOURCES  stm32_application/*.c FatFS/*.c)
file(GLOB_RECURSE USER_HEADERS  stm32_application/*.h stm32_application/images/*.h FatFS/*.h)

add_executable(${PROJECT_NAME}.elf ${STM_HEADERS} ${USER_SOURCES} ${USER_HEADERS} ${LINKER_SCRIPT}  stm32_application/usart.c stm32_application/usart.h stm32_application/init_stm32.c stm32_application/init_stm32.h stm32_application/support_functions.c stm32_application/support_functions.h stm32_application/painter.c stm32_application/painter.h stm32_application/fontem.h stm32_application/usart_dma.c stm32_application/usart_dma.h stm32_application/sdcard.h stm32_application/sdcard.c stm32f10x/stm32f10x_sdio.c stm32f10x/include/stm32f10x/stm32f10x_sdio.h stm32_application/json_parser.c stm32_application/json_parser.h stm32_application/build_calendar.c stm32_application/build_calendar.h stm32_application/global.h stm32_application/ini.c stm32_application/ini.h FatFS/ff.h FatFS/ff.c stm32_application/read_config.c stm32_application/read_config.h)

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