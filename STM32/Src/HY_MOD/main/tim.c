#include "HY_MOD/main/tim.h"

uint32_t tim_clk_APB1, tim_clk_APB2;

void INIT_OWN_TIM(void)
{
    tim_clk_APB1 = HAL_RCC_GetPCLK1Freq();
    tim_clk_APB2 = HAL_RCC_GetPCLK2Freq();
#ifdef STM32G431RB
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) tim_clk_APB1 *= 2U;
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1) tim_clk_APB2 *= 2U;
#elifdef STM32H753ZI
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t timpre = (RCC->CFGR & RCC_CFGR_TIMPRE);

    uint32_t ppre1 = (RCC->D2CFGR & RCC_D2CFGR_D2PPRE1);
    if (timpre == 0)
    {
        if (ppre1 != RCC_D2CFGR_D2PPRE1_DIV1) tim_clk_APB1 *= 2U;
    }
    else
    {
        if (ppre1 == RCC_D2CFGR_D2PPRE1_DIV1 || 
            ppre1 == RCC_D2CFGR_D2PPRE1_DIV2 || 
            ppre1 == RCC_D2CFGR_D2PPRE1_DIV4)
        {
            tim_clk_APB1 = hclk;
        }
        else
        {
            tim_clk_APB1 *= 4U;
        }
    }

    uint32_t ppre2 = (RCC->D2CFGR & RCC_D2CFGR_D2PPRE2);
    if (timpre == 0)
    {
        if (ppre2 != RCC_D2CFGR_D2PPRE2_DIV1) tim_clk_APB2 *= 2U;
    }
    else
    {
        if (ppre2 == RCC_D2CFGR_D2PPRE2_DIV1 || 
            ppre2 == RCC_D2CFGR_D2PPRE2_DIV2 || 
            ppre2 == RCC_D2CFGR_D2PPRE2_DIV4)
        {
            tim_clk_APB2 = hclk;
        }
        else
        {
            tim_clk_APB2 *= 4U;
        }
    }
#endif
}
