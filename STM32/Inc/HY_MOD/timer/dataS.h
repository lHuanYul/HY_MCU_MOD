#pragma once

#define TIM1_ARR_MAX    65535U
#define TIM2_ARR_MAX    4294967295U
#define TIM3_ARR_MAX    65535U
#define TIM4_ARR_MAX    65535U
#define TIM5_ARR_MAX    4294967295U
#define TIM6_ARR_MAX    65535U
#define TIM7_ARR_MAX    65535U
#define TIM8_ARR_MAX    65535U
#define TIM12_ARR_MAX   65535U
#define TIM13_ARR_MAX   65535U
#define TIM14_ARR_MAX   65535U
#define TIM15_ARR_MAX   65535U
#define TIM16_ARR_MAX   65535U
#define TIM17_ARR_MAX   65535U

// #define TIMx_ARR_MAX(instance) \
//     (((instance) == TIM2 || (instance) == TIM5) ? 4294967295U : 65535U)

#define TIMER_GET_CCR_ADDR(__INSTANCE__, __CHANNEL__) \
    (((__CHANNEL__) == TIM_CHANNEL_1) ? &((__INSTANCE__)->CCR1) : \
    ((__CHANNEL__) == TIM_CHANNEL_2) ? &((__INSTANCE__)->CCR2) : \
    ((__CHANNEL__) == TIM_CHANNEL_3) ? &((__INSTANCE__)->CCR3) : \
    ((__CHANNEL__) == TIM_CHANNEL_4) ? &((__INSTANCE__)->CCR4) : \
    ((__CHANNEL__) == TIM_CHANNEL_5) ? &((__INSTANCE__)->CCR5) : \
                                        &((__INSTANCE__)->CCR6))
