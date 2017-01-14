#ifndef LEDS_H_
#define LEDS_H_

typedef enum
{
  LED_GREEN = 0,
  LED_ORANGE = 1,
  LED_RED = 2,
  LED_BLUE = 3
}
Led_TypeDef;

// Function prototypes
void leds_init();
void leds_set(Led_TypeDef led);
void leds_clear(Led_TypeDef led);
void leds_toggle(Led_TypeDef led);
void leds_error();

#endif
