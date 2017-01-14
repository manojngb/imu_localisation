
############################# PROJECT SETUP #############################
# Put your stlink folder here so make burn will work.
STLINK=/usr/local/bin/stlink

# Add support for Cortex-M4 and STM32F4 Discovery Board
LIB_COMMON = ../common

# discovery support
DISCOVERY = $(LIB_COMMON)/stm32f3discovery/Libraries

# Project name
PROJ_NAME=ntb_watch
OUTPATH=build

############################# SOURCE DIRECTORIES ##########################
SRC_DIR = .
SRC_DIR += ntb/src
SRC_DIR += $(DISCOVERY)/STM32F30x_StdPeriph_Driver/src
SRC_DIR += $(LIB_COMMON)/decaranging/decadriver/src
SRC_DIR += $(LIB_COMMON)/decaranging/ranging/src
SRC_DIR += dw_platform/src

############################# INCLUDE DIRECTORIES ##########################
INC_DIR = .
INC_DIR += dw_platform/inc
INC_DIR += ntb/inc
INC_DIR += $(DISCOVERY)/CMSIS/Device/ST/STM32F30x/Include
INC_DIR += $(DISCOVERY)/CMSIS/Include
INC_DIR += $(DISCOVERY)/STM32F30x_StdPeriph_Driver/inc
INC_DIR += $(LIB_COMMON)/decaranging/decadriver/inc
INC_DIR += $(LIB_COMMON)/decaranging/ranging/inc

############################# ADDING C FILES #############################
SRCS = $(shell find $(SRC_DIR) -maxdepth 1 -name '*.c' -printf "%f\n")

############################# STARTUP FILE #############################

# add startup file to build
SRCS += ../common/startup/startup_stm32f30x.s 

############################# BUILD SETUP #############################
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

# Choose debug or release...
CFLAGS = -g -O2   # Normal

CFLAGS += -Tstm32f3.ld
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4

############################# PATH SETUP #############################
vpath %.c $(SRC_DIR) $(SRC_USB)

# Includes
CFLAGS += $(foreach d, $(INC_DIR), -I$d)
OBJS = $(SRCS:.c=.o)
# link math
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