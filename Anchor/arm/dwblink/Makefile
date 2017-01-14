
############################# PROJECT SETUP #############################
# Put your stlink folder here so make burn will work.
STLINK=/usr/local/bin/stlink

# Add support for Cortex-M4 and STM32F4 Discovery Board
STM_COMMON = ../common

# Project name
PROJ_NAME=stm32f4_dwblink
OUTPATH=build

############################# ADDING C FILES #############################
# Main sources
SRCS = $(wildcard *.c)

# USB sources
SRCS += $(wildcard vcp/src/*.c)

# LED Config
SRCS += $(wildcard leds/src/*.c)

# Decawave
SRCS += $(wildcard dw/decadriver/src/*.c)
SRCS += $(wildcard dw/application/src/*.c)

# STM32F4 library code
SRCS += stm32f4xx_exti.c stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_usart.c  stm32f4xx_rtc.c misc.c stm32f4xx_syscfg.c
SRCS += tm_stm32f4_usb_vcp.c usb_bsp.c usb_core.c usb_dcd.c
SRCS += usbd_cdc_core.c usbd_cdc_vcp.c usb_dcd_int.c usbd_core.c usbd_desc.c usbd_ioreq.c usbd_req.c usbd_usr.c
SRCS += usb_vcp_api.c
SRCS += stm32f4xx_spi.c

############################# STARTUP FILE #############################

# add startup file to build
SRCS += $(STM_COMMON)/startup/startup_stm32f4xx.s 

############################# BUILD SETUP #############################
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

# Choose debug or release...
CFLAGS = -g -O2           # Normal
#CFLAGS = -ggdb -O0       # RSW - for GDB debugging, disable optimizer

CFLAGS += -Tstm32_flash.ld
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4
CFLAGS += -msoft-float

############################# PATH SETUP #############################
vpath %.c \
$(STM_COMMON)/stm32f4discovery/Libraries/STM32F4xx_StdPeriph_Driver/src \
$(STM_COMMON)/stm32f4discovery/Libraries/STM32_USB_OTG_Driver/src \
$(STM_COMMON)/stm32f4discovery/Libraries/STM32_USB_Device_Library/Core/src \
$(STM_COMMON)/stm32f4discovery/Libraries/STM32_USB_Device_Library/Class/cdc/src \
vcp/src \
leds/src \
dw/decadriver/src \
dw/application/src

# Includes
CFLAGS += -I.
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/CMSIS/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/CMSIS/Include
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/STM32F4xx_StdPeriph_Driver/inc
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/STM32_USB_OTG_Driver/inc
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/STM32_USB_Device_Library/Core/inc
CFLAGS += -I$(STM_COMMON)/stm32f4discovery/Libraries/STM32_USB_Device_Library/Class/cdc/inc
CFLAGS += -Ivcp/inc
CFLAGS += -Ileds/inc
CFLAGS += -Idw/decadriver/inc
CFLAGS += -Idw/application/inc

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
