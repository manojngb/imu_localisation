#include <stdint.h>
#include "stm32f30x.h"
#include "ntb_hwctrl.h"

#define MAX_32BIT_INT (4294967296)

// Currently RNG is not used
void dw_rng_init()
{
	return;
}


float dw_rng_float()
{
	return 0.0;
}
