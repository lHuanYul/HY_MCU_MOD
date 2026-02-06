#include "HY_MOD/motor/ctrl_deg.h"
#ifdef HY_MOD_STM32_MOTOR

#define HIGH_PASS   1
#define NONE_PASS   0
#define LOW_PASS   -1

static const int8_t seq_map_120[][3] = {
    { HIGH_PASS, NONE_PASS, LOW_PASS  }, // 0-4
    { NONE_PASS, HIGH_PASS, LOW_PASS  }, // 1-6
    { LOW_PASS,  HIGH_PASS, NONE_PASS }, // 2-2
    { LOW_PASS,  NONE_PASS, HIGH_PASS }, // 3-3
    { NONE_PASS, LOW_PASS,  HIGH_PASS }, // 4-1
    { HIGH_PASS, LOW_PASS,  NONE_PASS }, // 5-5
    { HIGH_PASS, HIGH_PASS, HIGH_PASS }, // 6
    { LOW_PASS,  LOW_PASS,  LOW_PASS  }, // 7
};
static const uint8_t index_120_ccw[] = {7, 4, 2, 3, 0, 5, 1, 7};
static const uint8_t index_120_cw[]  = {7, 1, 5, 0, 3, 2, 4, 7};

static const int8_t seq_map_180[][3] = {
    { HIGH_PASS, LOW_PASS,  HIGH_PASS }, // 0-4
    { HIGH_PASS, LOW_PASS,  LOW_PASS  }, // 1-6
    { HIGH_PASS, HIGH_PASS, LOW_PASS  }, // 2-2
    { LOW_PASS,  HIGH_PASS, LOW_PASS  }, // 3-3
    { LOW_PASS,  HIGH_PASS, HIGH_PASS }, // 4-1
    { LOW_PASS,  LOW_PASS,  HIGH_PASS }, // 5-5
    { HIGH_PASS, HIGH_PASS, HIGH_PASS }, // 6
    { LOW_PASS,  LOW_PASS,  LOW_PASS  }, // 7
};
static const uint8_t index_180_lock[] = {7, 4, 2, 3, 0, 5, 1, 7};
static const uint8_t index_180_ccw[]  = {7, 0, 4, 5, 2, 1, 3, 7};
static const uint8_t index_180_cw[]   = {7, 2, 0, 1, 4, 3, 5, 7};

#define TIM_CH_ENABLE(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC1NE)) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER |= (TIM_CCER_CC2E | TIM_CCER_CC2NE)) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER |= (TIM_CCER_CC3E | TIM_CCER_CC3NE)) : 0)

#define TIM_CH_DISABLE(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1NE)) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER &= ~(TIM_CCER_CC2E | TIM_CCER_CC2NE)) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER &= ~(TIM_CCER_CC3E | TIM_CCER_CC3NE)) : 0)

#define TIM_PWM_ENABLE(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER |= TIM_CCER_CC1E) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER |= TIM_CCER_CC2E) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER |= TIM_CCER_CC3E) : 0)

#define TIM_PWM_DISABLE(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER &= ~TIM_CCER_CC1E) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER &= ~TIM_CCER_CC2E) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER &= ~TIM_CCER_CC3E) : 0)

#define TIM_PWMN_ENABLE(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER |= TIM_CCER_CC1NE) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER |= TIM_CCER_CC2NE) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER |= TIM_CCER_CC3NE) : 0)

#define TIM_PWMN_DISABLE(__HANDLE__, __CHANNEL__) \
  (((__CHANNEL__) == TIM_CHANNEL_1) ? ((__HANDLE__)->Instance->CCER &= ~TIM_CCER_CC1NE) : \
   ((__CHANNEL__) == TIM_CHANNEL_2) ? ((__HANDLE__)->Instance->CCER &= ~TIM_CCER_CC2NE) : \
   ((__CHANNEL__) == TIM_CHANNEL_3) ? ((__HANDLE__)->Instance->CCER &= ~TIM_CCER_CC3NE) : 0)

void deg_ctrl_120_load(MotorParameter *motor, uint8_t id)
{
    uint8_t i;
    float32_t *duty[3] = {
        &motor->pwm_duty_u,
        &motor->pwm_duty_v,
        &motor->pwm_duty_w
    };
    int8_t seq[3] = {0};
    switch (motor->mode_rot_ref)
    {
        case MOTOR_ROT_COAST:
        {
            motor->deg_duty = 1.0f;
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[6][i];
            break;
        }
        case MOTOR_ROT_BREAK:
        case MOTOR_ROT_LOCK:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[7][i];
            break;
        }
        case MOTOR_ROT_NORMAL:
        {
            for (i = 0; i < 3; i++)
            {
                if (!motor->rpm_reference.reverse)
                    seq[i] = seq_map_120[index_120_ccw[id]][i];
                else
                    seq[i] = seq_map_120[ index_120_cw[id]][i];
            }
            break;
        }
        case MOTOR_ROT_LOCK_FIN:
        {
            motor->deg_duty = 0.2f;
            // !
            for (i = 0; i < 3; i++)
                seq[i] = seq_map_120[index_180_lock[id]][i];
            break;
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (seq[i] == HIGH_PASS)
        {
            // TIM_CH_ENABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            TIM_PWM_ENABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            TIM_PWMN_DISABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            *duty[i] = motor->deg_duty;
        }
        else if (seq[i] == LOW_PASS)
        {
            // TIM_CH_ENABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            TIM_PWM_DISABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            TIM_PWMN_ENABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            *duty[i] = 1;
        }
        else
        {
            TIM_CH_DISABLE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
            *duty[i] = 0.87f;
        }
    }
}

void deg_ctrl_180_load(MotorParameter *motor)
{
    if (motor->hall_current == UINT8_MAX) return;
    uint8_t i;
    float32_t *duty[3] = {
        &motor->pwm_duty_u,
        &motor->pwm_duty_v,
        &motor->pwm_duty_w
    };
    int8_t seq[3] = {0};
    switch (motor->mode_rot_ref)
    {
        case MOTOR_ROT_COAST:
        {
            motor->deg_duty = 1.0f;
            for (i = 0; i < 3; i++) seq[i] = seq_map_180[6][i];
            break;
        }
        case MOTOR_ROT_BREAK:
        case MOTOR_ROT_LOCK:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_180[7][i];
            break;
        }
        case MOTOR_ROT_NORMAL:
        {
            for (i = 0; i < 3; i++)
            {
                if (!motor->rpm_reference.reverse)
                    seq[i] = seq_map_180[index_180_ccw[motor->hall_current]][i];
                else
                    seq[i] = seq_map_180[ index_180_cw[motor->hall_current]][i];
            }
            break;
        }
        case MOTOR_ROT_LOCK_FIN:
        {
            motor->deg_duty = 0.2f;
            for (i = 0; i < 3; i++)
                seq[i] = seq_map_180[index_180_lock[motor->hall_current]][i];
            break;
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (seq[i] == HIGH_PASS)
        {
            *duty[i] = motor->deg_duty;
        }
        else
        {
            *duty[i] = 0;
        }
    }
}

#endif