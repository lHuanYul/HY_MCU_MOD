#include "HY_MOD/main/main.h"

#include "HY_MOD/main/tim.h"

void hymod_init(void)
{
    hymod_init_tim();
}

void hymod_main(void)
{
    // If FreeRTOS, nothing should be here
    while (1)
    {
    }
}
