INCLUDE(CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/stm32_application/link.ld)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
#change -O flat to 2, back or
# SET(COMMON_FLAGS "-ffunction-sections -fdata-sections -O2 -ggdb  -mcpu=cortex-m3 -mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float -mfix-cortex-m3-ldrd")
SET(COMMON_FLAGS "-specs=nano.specs -specs=nosys.specs -ffunction-sections -fdata-sections -Og -ggdb  -mcpu=cortex-m3 -mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float -mfix-cortex-m3-ldrd")

SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++11")
#SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99 -D__TM_GMTOFF")
SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99 -D_XOPEN_SOURCE=700 -D_TM_GMTOFF=1")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-gc-sections -T ${LINKER_SCRIPT}")