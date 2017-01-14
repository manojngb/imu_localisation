#include <unistd.h>
#include <stdio.h>
#include "leds.h"

// LED functions
void leds_set(Led_TypeDef led)
{
  FILE *fp;
  switch (led)
  {
  case LED_GREEN:
    fp = fopen("/sys/class/leds/beaglebone:green:heartbeat/brightness", "w");
    break;
  case LED_ORANGE:
    fp = fopen("/sys/class/leds/beaglebone:green:mmc0/brightness", "w");
    break;
  case LED_RED:
    fp = fopen("/sys/class/leds/beaglebone:green:usr2/brightness", "w");
    break;
  case LED_BLUE:
    fp = fopen("/sys/class/leds/beaglebone:green:usr3/brightness", "w");
    break;
  default:
    return;
    break;
  }
  fprintf(fp, "%d", 1);
  fclose(fp);
}

void leds_clear(Led_TypeDef led)
{
  FILE *fp;
  switch (led)
  {
  case LED_GREEN:
    fp = fopen("/sys/class/leds/beaglebone:green:heartbeat/brightness", "w");
    break;
  case LED_ORANGE:
    fp = fopen("/sys/class/leds/beaglebone:green:mmc0/brightness", "w");
    break;
  case LED_RED:
    fp = fopen("/sys/class/leds/beaglebone:green:usr2/brightness", "w");
    break;
  case LED_BLUE:
    fp = fopen("/sys/class/leds/beaglebone:green:usr3/brightness", "w");
    break;
  default:
    return;
    break;
  }
  fprintf(fp, "%d", 0);
  fclose(fp);
}

void leds_toggle(Led_TypeDef led)
{
  FILE *fp;
  int val;
  switch (led)
  {
  case LED_GREEN:
    fp = fopen("/sys/class/leds/beaglebone:green:heartbeat/brightness", "r");
    fscanf(fp, "%d", &val);
    fclose(fp);
    fp = fopen("/sys/class/leds/beaglebone:green:heartbeat/brightness", "w");
    fprintf(fp, "%d", (val + 1) % 2);
    break;
  case LED_ORANGE:
    fp = fopen("/sys/class/leds/beaglebone:green:mmc0/brightness", "r");
    fscanf(fp, "%d", &val);
    fclose(fp);
    fp = fopen("/sys/class/leds/beaglebone:green:mmc0/brightness", "w");
    fprintf(fp, "%d", (val + 1) % 2);
    break;
  case LED_RED:
    fp = fopen("/sys/class/leds/beaglebone:green:usr2/brightness", "r");
    fscanf(fp, "%d", &val);
    fclose(fp);
    fp = fopen("/sys/class/leds/beaglebone:green:usr2/brightness", "w");
    fprintf(fp, "%d", (val + 1) % 2);
    break;
  case LED_BLUE:
    fp = fopen("/sys/class/leds/beaglebone:green:usr3/brightness", "r");
    fscanf(fp, "%d", &val);
    fclose(fp);
    fp = fopen("/sys/class/leds/beaglebone:green:usr3/brightness", "w");
    fprintf(fp, "%d", (val + 1) % 2);
    break;
  default:
    return;
    break;
  }
  fclose(fp);                       // Close
}

void leds_error()
{
  int i;
  while (1)
  {
    leds_clear(LED_GREEN);
    leds_clear(LED_ORANGE);
    leds_clear(LED_RED);
    leds_clear(LED_BLUE);
    usleep(500000);
    leds_set(LED_GREEN);
    leds_set(LED_ORANGE);
    leds_set(LED_RED);
    leds_set(LED_BLUE);
    usleep(500000);
  }
}

//Configure pins and clocks
void leds_init()
{
  leds_clear(LED_GREEN);
  leds_clear(LED_RED);
  leds_clear(LED_ORANGE);
  leds_clear(LED_BLUE);
}
