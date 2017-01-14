#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define MAX_32BIT_INT (4294967296)

void dw_rng_init()
{
	srand(time(NULL));
}

float dw_rng_float()
{
	return ((float)rand())/MAX_32BIT_INT;
}
