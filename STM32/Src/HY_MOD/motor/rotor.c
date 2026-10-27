#include "HY_MOD/motor/rotor.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/buffer.h"
#include "HY_MOD/motor/ctrl.h"

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

void motor_set_rotor_mode(MotorParameter *motor, MotorSensorMode mode)
{
    volatile MotorSensorMode *m = &motor->rotor.mode;
    if ((*m) == mode) return;
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
    (*m) = mode;
}

void motor_rotor_hall_tim_tri(MotorParameter *motor)
{
    switch (motor->rotor.mode)
    {
        case MOTOR_SENSOR_SIMULATE:
        case MOTOR_SENSOR_LESS_VOLTAGE:
        case MOTOR_SENSOR_LESS_CURRENT:
        {
            FLAGS_SET(*(motor->system.Hall_Addr_IT), TIM_EGR_CC1G | TIM_EGR_UG);
            break;
        }
        case MOTOR_SENSOR_UNINIT:
        case MOTOR_SENSOR_HALL_EXTI:
        case MOTOR_SENSOR_HALL_PWM_T:
        default: break;
    }
}

static const uint8_t angle_hall_to_pu[8] = {MOTOR_HALL_OF, 4, 2, 3, 0, 5, 1, MOTOR_HALL_OF};
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

inline void motor_rotor_set_overflow(MotorParameter *motor, uint32_t of)
{
    motor->rotor.tim_ar = of;
    __HAL_TIM_SET_AUTORELOAD(motor->system.const_h.Hall_htimx, of);
}

/* ---------- Proccess ---------- */

void motor_rotor_I_F_trigger(MotorParameter *motor)
{
    MotorRotorParam *r = &motor->rotor;
    if (motor->tim_tick % r->I_F_trigger_T == 0)
    {
        volatile uint8_t *p = &r->virtual;
        (*p) = motor_deg_get_next_phase(motor, (*p));
        motor_rotor_hall_tim_tri(motor);
    }
}

/**
 * @brief 換相
 */
static void phase_update(MotorParameter *motor, uint8_t phase)
{
    if (phase == MOTOR_HALL_OF) return;
    MotorRotorParam *r = &motor->rotor;
    if (phase == r->curr) return;
    r->prev = r->curr;
    r->curr = phase;
    motor->tim_tick_ch_phase = 0;
}

static void sensorless_V(MotorParameter *motor)
{
    // 1. 若已經偵測到 ZCP，進入 30 度換相倒數階段
    if (motor->zcp.zcp_detected)
    {
        if (motor->zcp.delay_30deg_cnt > 0)
        {
            motor->zcp.delay_30deg_cnt--;
        }
        else
        {
            // 30 度倒數結束 -> 正式觸發換相！
            motor->zcp.zcp_detected = 0;
            motor->zcp.comm_ticks = 0;
            motor->zcp.zcp_cnt = 0;
            
            phase_update(motor, motor_deg_get_next_phase(motor, motor->rotor.curr));
        }
        return;
    }

    // 5. 連續次數確認 (防單次開關雜訊)
    if (motor_deg_zcp_match(motor))
    {
        motor->zcp.zcp_cnt++;
        if (motor->zcp.zcp_cnt >= ZCP_CONFIRM_TIMES)
        {
            // ZCP 正式成立！
            motor->zcp.zcp_detected = 1;
            motor->zcp.zcp_cnt = 0;

            // 60度耗時等於 comm_ticks，因此 30度延遲約為 comm_ticks的一半
            // 扣除掉連續確認已消耗的 2 個 tick
            int32_t delay = (motor->tim_tick_ch_phase >> 1) - ZCP_CONFIRM_TIMES;
            motor->zcp.delay_30deg_cnt = (delay > 0) ? (uint16_t)delay : 0;
        }
    }
    else
    {
        // 中斷連續性，雜訊清零
        motor->zcp.zcp_cnt = 0;
    }
}

static void time_upd(MotorParameter *motor, uint32_t time)
{
    uint8_t reverse = 0;
    MotorRotorParam *r = &motor->rotor;
    MotorSpeedParame *s = &motor->speed;
    #define ROTOR_HISTORY_STORE() \
        HISTORY_STORE_SUM( \
            r->times_histoy.datas, \
            r->times_histoy.head, \
            r->times_histoy.len, \
            MOTOR_SPD_CNT, \
            r->times_histoy.sum, \
            time \
        )
    if (r->curr == r->prev)
    {
        // Todo
        if (s->fbk_omega < 0) reverse = 1;
    }
    else if (r->curr == ((r->prev + 1) % 6))
    {
        r->wrong = 0;
        ROTOR_HISTORY_STORE();
    }
    else if (r->curr == ((r->prev + 5) % 6))
    {
        r->wrong = 0;
        ROTOR_HISTORY_STORE();
        reverse = 1;
    }
    else
    {
        r->wrong++;
        if (r->wrong >= 3)
        {
            if (r->wrong >= 253) r->wrong = 3;
            // omega = 0.0f;
            // motor->foc_h.rad_itpl = 0.0f;
        }

        HISTORY_STORE(motor->dbg_h.hall_wrong, motor->dbg_h.hall_wrong_c,
            20, r->prev * 10 + r->curr);
    }
    uint32_t total = r->times_histoy.sum;
    if (total == 0) return;
    float32_t total_i = VAR_ONE_DIV((float32_t)total);
    float32_t omega =
        r->times_histoy.len * motor->system.omega_fbk * total_i;
    if (reverse) omega *= -1.0f;
    s->fbk_omega    = omega;
    s->fbk_rpm      = omega * OMEGA_TO_RPM;

    motor->foc_h.rad_itpl       = (omega >= 0.0f ? 1.0f : -1.0f) *
        (r->times_histoy.len * motor->system.foc_it_angle_itpl * total_i);
}

void motor_rotor_hall_timer_cbi(MotorParameter *motor)
{
    MotorRotorParam *r = &motor->rotor;
    uint32_t time = *(motor->system.Hall_Addr_CCR);
    switch (r->mode)
    {
        case MOTOR_SENSOR_UNINIT: return;
        case MOTOR_SENSOR_SIMULATE:
        {
            phase_update(motor, r->virtual);
            break;
        }
        case MOTOR_SENSOR_HALL_EXTI:
        {
            phase_update(motor, motor_rotor_hall_get(motor));
            break;
        }
        default: break;
    }
    time_upd(motor, time);
}

void motor_rotor_pwm_cbi(MotorParameter *motor)
{
    switch (motor->rotor.mode)
    {
        case MOTOR_SENSOR_UNINIT: return;
        case MOTOR_SENSOR_HALL_PWM_T:
        {
            phase_update(motor, motor_rotor_hall_get(motor));
            break;
        }
        case MOTOR_SENSOR_LESS_VOLTAGE:
        {
            sensorless_V(motor);
            break;
        }
        case MOTOR_SENSOR_LESS_CURRENT:
        {
            // sensorless_I(motor);
            break;
        }
        default: return;
    }
}

/* ---------- Stop ---------- */

void motor_rotor_stop_cbi(MotorParameter *motor)
{
    MotorRotorParam *r = &motor->rotor;
    r->stop_tick = HAL_GetTick();
    r->times_histoy     = (typeof(r->times_histoy)){0};
    r->prev      = r->curr;
    MotorSpeedParame *s = &motor->speed;
    s->fbk_omega = 0.0f;
    s->fbk_rpm   = 0.0f;
}

#endif
