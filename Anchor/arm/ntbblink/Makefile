
############################# PROJECT SETUP #############################
# Put your stlink folder here so make burn will work.
STLINK=/usr/local/bin/stlink

# Add support for Cortex-M4 and STM32F4 Discovery Board
STM_COMMON = ../common

# Project name
PROJ_NAME=ntb_blink
OUTPATH=build

############################# ADDING C FILES #############################
# Main sources
SRCS = $(wildcard *.c)

# STM32F4 library code
SRCS += stm32f4xx_exti.c stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_usart.c  stm32f4xx_rtc.c misc.c stm32f4xx_syscfg.c

############################# STARTUP FILE #############################

# add startup file to build
SRCS += $(STM_COMMON)/startup/startup_stm32f4xx.s 

############################# BUILD SETUP #############################
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

# Choose debug or release...
CFLAGS = -g -O2           # Normal

CFLAGS += -Tstm32_flash.ld
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4
CFLAGS += -msoft-float

############################# PATH SETUP #############################
vpath %.c \
$(STM_COMMON)/stm32f4discovery/Libraries/STM32F4xx_StdPeriph_Driver/src

# Includes
CFLAGS += -I.
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/CMSIS/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/CMSIS/Include
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/STM32F4xx_StdPeriph_Driver/inc

OBJS = $(SRCS:.c=.o)

################################ TARGETS ################################
.PHONY: lib proj dir_tree

all:  dir_tree proj

dir_tree:
	mkdir -p $(OUTPATH)

proj: $(OUTPATH)/$(PROJ_NAME).elf
	$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBPATHS) $(LIBS)
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

dfu: proj
	dfu-util -v -d 0483:df11 -a 0 -s 0x08000000 -D build/ntb_blink.bin
