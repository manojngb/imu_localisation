
############################# PROJECT SETUP #############################
# Put your stlink folder here so make burn will work.
STLINK=/usr/local/bin/stlink

# Add support for Cortex-M4 and STM32F4 Discovery Board
LIB_COMMON = ../common

# discovery support
DISCOVERY = $(LIB_COMMON)/stm32f4discovery/Libraries

# lwip support
LWIP = $(LIB_COMMON)/lwip_v1.3.2

# Project name
PROJ_NAME=ntb_anchor
OUTPATH=build

############################# SOURCE DIRECTORIES ##########################
SRC_DIR = .
SRC_DIR += leds/src
SRC_DIR += eth/src
SRC_DIR += strlib/src
SRC_DIR += ntb/src
SRC_DIR += $(LWIP)/port/STM32F4x7/Standalone
SRC_DIR += $(LWIP)/src/api
SRC_DIR += $(LWIP)/src/core
SRC_DIR += $(LWIP)/src/core/ipv4
SRC_DIR += $(LWIP)/src/core/snmp
SRC_DIR += $(LWIP)/src/netif
SRC_DIR += $(LWIP)/src/netif/ppp
SRC_DIR += $(DISCOVERY)/STM32F4xx_StdPeriph_Driver/src
SRC_DIR += $(LIB_COMMON)/STM32F4x7_ETH_Driver/src
SRC_DIR += $(LIB_COMMON)/decaranging/decadriver/src
SRC_DIR += $(LIB_COMMON)/decaranging/ranging/src
SRC_DIR += dw_platform/src

############################# INCLUDE DIRECTORIES ##########################
INC_DIR = .
INC_DIR += leds/inc
INC_DIR += eth/inc
INC_DIR += strlib/inc
INC_DIR += dw_platform/inc
INC_DIR += ntb/inc
INC_DIR += $(LWIP)/port/STM32F4x7/Standalone
INC_DIR += $(LWIP)/src/include
INC_DIR += $(LWIP)/port/STM32F4x7
INC_DIR += $(LWIP)/src/include/ipv4
INC_DIR += $(LWIP)/src/include/netif
INC_DIR += $(DISCOVERY)/CMSIS/ST/STM32F4xx/Include
INC_DIR += $(DISCOVERY)/CMSIS/Include
INC_DIR += $(DISCOVERY)/STM32F4xx_StdPeriph_Driver/inc
INC_DIR += $(LIB_COMMON)/STM32F4x7_ETH_Driver/inc
INC_DIR += $(LIB_COMMON)/stm32f4discovery/Utilities/STM32F4-Discovery
INC_DIR += $(LIB_COMMON)/decaranging/decadriver/inc
INC_DIR += $(LIB_COMMON)/decaranging/ranging/inc

############################# ADDING C FILES #############################
SRCS = $(shell find $(SRC_DIR) -maxdepth 1 -name '*.c' -printf "%f\n")

############################# STARTUP FILE #############################

# add startup file to build
SRCS += ../common/startup/startup_stm32f4xx.s 

############################# BUILD SETUP #############################
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

# Choose debug or release...
CFLAGS = -g -O2   # Normal

CFLAGS += -Tstm32_iap.ld
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4
#CFLAGS += -msoft-float

############################# PATH SETUP #############################
vpath %.c $(SRC_DIR) $(SRC_USB)

# Includes
CFLAGS += $(foreach d, $(INC_DIR), -I$d)
OBJS = $(SRCS:.c=.o)
LDFLAGS = -lm

################################ TARGETS ################################
.PHONY: lib proj dir_tree

all:  dir_tree proj

dir_tree:
	mkdir -p $(OUTPATH)

proj: $(OUTPATH)/$(PROJ_NAME).elf
	$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBPATHS) $(LIBS) $(LDFLAGS)
	$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin

clean:
	find . -name \*.o -type f -delete
	find . -name \*.lst -type f -delete
	rm -f $(OUTPATH)/$(PROJ_NAME).elf
	rm -f $(OUTPATH)/$(PROJ_NAME).hex
	rm -f $(OUTPATH)/$(PROJ_NAME).bin

# Flash the STM32F4
burn: proj
	$(STLINK)/st-flash write $(OUTPATH)/$(PROJ_NAME).bin 0x8000000

dfu:
	dfu-util -v -d 0483:df11 -a 0 -s 0x08000000 -D $(OUTPATH)/$(PROJ_NAME).bin

print-%:
	@echo $* = $($*)