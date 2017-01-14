# Minimum version of CMake required to build this file
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(dwrange)

# Cross-compile properties
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm) 
SET(CMAKE_SYSTEM_VERSION 1)

# Specify the cross compiler (need to work out )
SET(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
SET(CMAKE_FIND_ROOT_PATH /export/rootfs)

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Hints for finding boost in the rootfs
SET(BOOST_INCLUDEDIR "/export/rootfs/usr/include")
SET(BOOST_LIBRARYDIR "/export/rootfs/usr/lib/arm-linux-gnueabihf")

# We'll be using C++11 functionality in this project
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Preprocessor directive required to link boost trivial logging
FIND_PACKAGE(Boost 1.55 REQUIRED COMPONENTS thread system program_options)
FIND_PACKAGE(Threads REQUIRED)

# Location of header files for the entire project
INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../dw/decadriver/inc
  ../dw/ranging/inc
  ./dw_platform/inc/
  ./leds/inc/
  "/export/rootfs/usr/include/arm-linux-gnueabihf"
)

# Location of boost libraries
LINK_DIRECTORIES(
  ${Boost_LIBRARY_DIRS} 
  "/export/rootfs/usr/lib/arm-linux-gnueabihf" 
  "/export/rootfs/lib/arm-linux-gnueabihf"
)

# Create the dwrange library
add_library(dw
  ../dw/decadriver/inc/deca_device_api.h
  ../dw/decadriver/inc/deca_param_types.h
  ../dw/decadriver/inc/deca_regs.h
  ../dw/decadriver/inc/deca_version.h
  ../dw/ranging/inc/deca_ranging.h
  ./leds/inc/leds.h
  ./dw_platform/inc/deca_spi.h
  ./dw_platform/inc/deca_debug.h
  ./dw_platform/inc/deca_rng.h
  ../dw/decadriver/src/deca_device.c
  ../dw/decadriver/src/deca_params_init.c
  ../dw/decadriver/src/deca_range_tables.c
  ../dw/ranging/src/deca_ranging.c
  ./leds/src/leds.c
  ./dw_platform/src/deca_spi.c
  ./dw_platform/src/deca_debug.c
  ./dw_platform/src/deca_rng.c
)

add_executable(dwrange dwrange.c)
target_link_libraries(dwrange dw)

