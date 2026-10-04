#include "HY_MOD/motor/rotor.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/buffer.h"

/* ---------- Hall Sensor ---------- */

static const uint8_t angle_hall_to_pu[8] = {10, 4, 2, 3, 0, 5, 1, 10};

static void motor_rotor_hall_enable(MotorParameter *motor)
{
    TIM_TypeDef *hall = motor->system.const_h.Hall_htimx->Instance;
    // 關閉中斷
    FLAGS_CLR(hall->DIER, TIM_DIER_TIE);

    // 開啟 XOR 組合輸入
    FLAGS_SET(hall->CR2, TIM_CR2_TI1S);
    // 設定 Slave 模式：TS = 100b (TI1F_ED), SMS = 100b (Reset Mode)
    FLAGS_FULLSET(hall->SMCR,
        TIM_SMCR_SMS | TIM_SMCR_TS, TIM_SMCR_SMS_2 | TIM_SMCR_TS_2);

    // 清除旗標
    FLAGS_CLR(hall->SR, TIM_SR_TIF);
    // 重新啟用中斷
    FLAGS_SET(hall->DIER, TIM_DIER_TIE);
    // 確保 Timer 運作
    FLAGS_SET(hall->CR1, TIM_CR1_CEN);
}

static void motor_rotor_hall_disable(MotorParameter *motor)
{
    TIM_TypeDef *hall = motor->system.const_h.Hall_htimx->Instance;
    // 關閉中斷
    FLAGS_CLR(hall->DIER, TIM_DIER_TIE);

    // 解除 Slave Mode 與觸發源選擇
    FLAGS_CLR(hall->SMCR, TIM_SMCR_SMS | TIM_SMCR_TS);
    // 關閉 XOR 組合輸入
    FLAGS_CLR(hall->CR2, TIM_CR2_TI1S);

    // 清除旗標
    FLAGS_CLR(hall->SR, TIM_SR_TIF);
    // 新啟用中斷
    FLAGS_SET(hall->DIER, TIM_DIER_TIE);
    // 確保 Timer 運作
    FLAGS_SET(hall->CR1, TIM_CR1_CEN);
}

uint8_t motor_rotor_hall_get(MotorParameter *motor)
{
    const MotorConst *const_h = &motor->system.const_h;
    uint8_t hall =
          (GPIO_READ_R(const_h->Hall_a.gpio) ? 4U : 0U)
        | (GPIO_READ_R(const_h->Hall_b.gpio) ? 2U : 0U)
        | (GPIO_READ_R(const_h->Hall_c.gpio) ? 1U : 0U);
    hall = angle_hall_to_pu[hall];
    return hall;
}

/* ---------- SensorLess ---------- */

/* ---------- Proccess ---------- */

inline void motor_rotor_set_overflow(MotorParameter *motor, uint32_t of)
{
    motor->rotor.overflow = of;
    __HAL_TIM_SET_AUTORELOAD(motor->system.const_h.Hall_htimx, of);
}

void motor_set_rotor_mode(MotorParameter *motor, MotorSensorMode mode)
{
    if (motor->rotor.mode == mode) return;
    switch (mode)
    {
        case MOTOR_SENSOR_UNINIT: return;
        case MOTOR_SENSOR_SIMULATE:
        case MOTOR_SENSOR_LESS_VOLTAGE:
        case MOTOR_SENSOR_LESS_CURRENT:
        {
            motor_rotor_hall_disable(motor);
            break;
        }
        case MOTOR_SENSOR_HALL_EXTI:
        case MOTOR_SENSOR_HALL_PWM_T:
        {
            motor_rotor_hall_enable(motor);
            break;
        }
        default: break;
    }
    motor->rotor.mode = mode;
}

void motor_rotor_phase_trigger(MotorParameter *motor)
{
    switch (motor->rotor.mode)
    {
        case MOTOR_SENSOR_UNINIT: return;
        case MOTOR_SENSOR_SIMULATE:
        case MOTOR_SENSOR_LESS_VOLTAGE:
        case MOTOR_SENSOR_LESS_CURRENT:
        {
            FLAGS_SET(*(motor->system.Hall_Addr_IT), TIM_EGR_CC1G | TIM_EGR_UG);
            break;
        }
        default: break;
    }
}

static void phase_upd(MotorParameter *motor, uint8_t phase)
{
    if (
        (phase == UINT8_MAX) ||
        (phase == motor->rotor.curr)
    ) return;
    motor->rotor.prev = motor->rotor.curr;
    motor->rotor.curr = phase;
}

#define ROTOR_HISTORY_STORE() \
    HISTORY_STORE_SUM( \
        motor->rotor.times.datas, \
        motor->rotor.times.head, \
        motor->rotor.times.len, \
        MOTOR_SPD_CNT, \
        motor->rotor.times.sum, \
        time \
    )
static void time_upd(MotorParameter *motor, uint32_t time)
{
    uint8_t reverse = 0;
    if (motor->rotor.curr == motor->rotor.prev)
    {
        // Todo
        if (motor->speed.fbk_omega < 0) reverse = 1;
    }
    else if (motor->rotor.curr == ((motor->rotor.prev + 1) % 6))
    {
        motor->rotor.wrong = 0;
        ROTOR_HISTORY_STORE();
    }
    else if (motor->rotor.curr == ((motor->rotor.prev + 5) % 6))
    {
        motor->rotor.wrong = 0;
        ROTOR_HISTORY_STORE();
        reverse = 1;
    }
    else
    {
        motor->rotor.wrong++;
        if (motor->rotor.wrong >= 3)
        {
            if (motor->rotor.wrong >= 253) motor->rotor.wrong = 3;
            // omega = 0.0f;
            // motor->foc_h.rad_itpl = 0.0f;
        }

        HISTORY_STORE(motor->dbg_h.hall_wrong, motor->dbg_h.hall_wrong_c,
            20, motor->rotor.prev * 10 + motor->rotor.curr);
    }
    uint32_t total = motor->rotor.times.sum;
    if (total == 0) return;
    float32_t total_i = 1.0f / (float32_t)total;
    float32_t omega =
        motor->rotor.times.len * motor->system.omega_fbk * total_i;
    if (reverse) omega *= -1.0f;
    motor->speed.fbk_omega    = omega;
    motor->speed.fbk_rpm      = omega * OMEGA_TO_RPM;
    motor->foc_h.rad_itpl       = (omega >= 0.0f ? 1.0f : -1.0f) *
        (motor->rotor.times.len * motor->system.foc_it_angle_itpl * total_i);
}

void motor_rotor_hall_timer_cbi(MotorParameter *motor)
{
    uint32_t time = *(motor->system.Hall_Addr_CCR);
    switch (motor->rotor.mode)
    {
        case MOTOR_SENSOR_UNINIT: return;
        case MOTOR_SENSOR_SIMULATE:
        {

            phase_upd(motor, motor->rotor.virtual);
            break;
        }
        case MOTOR_SENSOR_HALL_EXTI:
        {
            phase_upd(motor, motor_rotor_hall_get(motor));
            break;
        }
        default: break;
    }
    time_upd(motor, time);
}

void motor_rotor_pwm_cbi(MotorParameter *motor)
{
    uint8_t phase = UINT8_MAX;
    switch (motor->rotor.mode)
    {
        case MOTOR_SENSOR_UNINIT: return;
        case MOTOR_SENSOR_HALL_PWM_T:
        {
            phase = motor_rotor_hall_get(motor);
            break;
        }
        case MOTOR_SENSOR_LESS_VOLTAGE:
        case MOTOR_SENSOR_LESS_CURRENT:
        {
            // Todo
            phase = motor_rotor_hall_get(motor);
            break;
        }
        default: return;
    }
    phase_upd(motor, phase);
}

/* ---------- Stop ---------- */

void motor_rotor_stop_cbi(MotorParameter *motor)
{
    motor->rotor.stop_tick = HAL_GetTick();
    motor->rotor.times = (typeof(motor->rotor.times)){0};
    motor->rotor.prev = motor->rotor.curr;
    motor->speed.fbk_omega = 0.0f;
    motor->speed.fbk_rpm = 0.0f;
}

#endif
