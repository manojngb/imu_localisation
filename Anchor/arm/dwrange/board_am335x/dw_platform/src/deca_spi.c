#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/poll.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "leds.h"
#include "deca_spi.h"

#define DEBUG 0

int fd;               // Descriptor
struct pollfd pfds;   // Poll

void dw_spi_init()
{
  static uint8_t mode = SPI_MODE;
  static uint8_t bits = SPI_BITS;
  static uint32_t speed = SPI_SPEED;

  fd = open(SPI_DEVICE, O_RDWR);
  if (fd < 1)
    return; 

  ioctl(fd, SPI_IOC_WR_MODE, &mode);
  ioctl(fd, SPI_IOC_RD_MODE, &mode);
  ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
}

void dw_spi_close()
{
  if (fd < 1) return;
  close(fd);
}

int dw_spi_sendpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body)
{
  if (fd < 1) return;
  int ret;
  struct spi_ioc_transfer tr[2];
  memset(tr,0x0,sizeof(tr));
  tr[0].tx_buf = (unsigned long) header;
  tr[0].rx_buf = (unsigned long) NULL;
  tr[0].len = headerLen;
  tr[0].delay_usecs = SPI_DELAY;
  tr[0].speed_hz = SPI_SPEED;
  tr[0].bits_per_word = SPI_BITS;
  tr[0].cs_change = 0;
  tr[1].tx_buf = (unsigned long) body;
  tr[1].rx_buf = (unsigned long) NULL;
  tr[1].len = bodyLen;
  tr[1].delay_usecs = SPI_DELAY;
  tr[1].speed_hz = SPI_SPEED;
  tr[1].bits_per_word = SPI_BITS;
  tr[1].cs_change = 1;
  ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
  //printf("R: %d, ",ret);
  return (ret < 0);
}

int dw_spi_readpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body)
{
  if (fd < 1) return;
  int ret;
  struct spi_ioc_transfer tr[2];
  memset(tr,0x0,sizeof(tr));
  tr[0].tx_buf = (unsigned long) header;
  tr[0].rx_buf = (unsigned long) NULL;
  tr[0].len = headerLen;
  tr[0].delay_usecs = SPI_DELAY;
  tr[0].speed_hz = SPI_SPEED;
  tr[0].bits_per_word = SPI_BITS;
  tr[0].cs_change = 0;
  tr[1].tx_buf = (unsigned long) NULL;
  tr[1].rx_buf = (unsigned long) body;
  tr[1].len = bodyLen;
  tr[1].delay_usecs = SPI_DELAY;
  tr[1].speed_hz = SPI_SPEED;
  tr[1].bits_per_word = SPI_BITS;
  tr[1].cs_change = 1;
  ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
  //printf("W: %d, ",ret);
  return (ret < 0);
}

uint8_t dw_spi_poll(void)
{
  int ret;
  if (fd < 1) 
    return;
  pfds.fd = fd;
  pfds.events = POLLIN;
  pfds.revents = 0;
  ret = poll(&pfds, 1, -1);
  return (pfds.revents & POLLIN && ret > 0);
}

void dw_sleep_usec(uint32_t usec)
{
	usleep(usec);
}

void dw_sleep_msec(uint32_t msec)
{
	dw_sleep_usec(1000*msec);
}