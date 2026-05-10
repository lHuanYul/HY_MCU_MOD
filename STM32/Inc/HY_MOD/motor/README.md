```text
MotorParameter
├── const_h (const MotorConst) // 常數
│   ├── Hall_GPIO (MotorPhaseGPIOData) // HALL PIN
│   │   ├── u (GPIOData)
│   │   ├── v (GPIOData)
│   │   └── w (GPIOData)
│   ├── PWM_htimx (TIM_HandleTypeDef*) // PWM timer
│   ├── PWM_tim_clk (uint32_t*)
│   ├── PWM_TIM_CH_x (MotorPhasePwmCH)
│   │   ├── u (uint32_t)
│   │   ├── v (uint32_t)
│   │   ├── w (uint32_t)
│   │   └── mid (uint32_t)
│   ├── PWMN_GPIO (MotorPhaseGPIOData)
│   │   ├── u (GPIOData)
│   │   ├── v (GPIOData)
│   │   └── w (GPIOData)
│   ├── PWMN_GPIO_set (MotorPhaseNPwmGPIO)
│   │   ├── u (MotorPwmNGpio)
│   │   │   ├── MODEx (uint32_t)
│   │   │   ├── MODEx_0 (uint32_t)
│   │   │   └── MODEx_1 (uint32_t)
│   │   ├── v (MotorPwmNGpio)
│   │   │   ├── MODEx (uint32_t)
│   │   │   ├── MODEx_0 (uint32_t)
│   │   │   └── MODEx_1 (uint32_t)
│   │   └── w (MotorPwmNGpio)
│   │       ├── MODEx (uint32_t)
│   │       ├── MODEx_0 (uint32_t)
│   │       └── MODEx_1 (uint32_t)
│   ├── Hall_htimx (TIM_HandleTypeDef*) // 霍爾計時器
│   ├── Hall_tim_clk (uint32_t*)
│   ├── rated_trorque (float32_t) // 馬達data sheet
│   ├── rated_current (float32_t)
│   ├── peak_trorque (float32_t)
│   └── peak_current (float32_t)
├── tfm_h (MotorTfm)
│   ├── omega_fbk (float32_t) // 霍爾間隔 → 輸出軸轉速(omega) 轉換常數
│   └── foc_it_angle_itpl (float32_t) // PWM 週期 → 電角度內插轉換常數
├── dbg_h (MotorDbg)
│   ├── pwm_freq (float32_t) // 計時器頻率
│   ├── current_zero (float32_t)
│   └── hall_rad[6] (float32_t)
├── init_cnt (uint32_t)
├── ctrl_h (MotorCtrlParameter) // 馬達控制模式(120度與foc以及細部)
│   ├── ref_ori (MotorCtrl)
│   └── ref_fix (MotorCtrl)
├── rotate_h (MotorRotParameter) // 馬達旋轉模式(滑行與剎車等)
│   ├── ref_ori (MotorRot)
│   └── ref_fix (MotorRot)
├── speed_h (MotorSpdParameter) // 從座往轉子 順時針為負
│   ├── ref_rpm (float32_t)
│   ├── ref_omega (float32_t)
│   ├── fbk_rpm (float32_t)
│   ├── fbk_omega (float32_t)
│   └── save_stop_omega (float32_t)
├── tim_tick (uint32_t) // 計時中斷計數
├── adc_h (MotorADC) // 電流 ADC
│   ├── u (AdcCurrentParameter*)
│   ├── v (AdcCurrentParameter*)
│   ├── w (AdcCurrentParameter*)
│   └── total (float32_t) // 應接近0
├── hall_h (MotorHallParameter)
│   ├── time_hist_head (volatile uint8_t) // 霍爾跳變間隔 頭id
│   ├── time_hist_len (volatile uint8_t) // 霍爾跳變間隔 長度
│   ├── time_hist[MOTOR_SPD_CNT] (uint32_t) // 霍爾跳變間隔時間
│   ├── current (volatile uint8_t) // 目前霍爾相位
│   ├── last (uint8_t) // 上次霍爾相位
│   ├── wrong (volatile uint8_t)
│   ├── auto_spin (uint8_t) // Todo
│   └── stop_tick (uint32_t) // 停轉時間
├── deg_h (MotorDEGParameter)
│   ├── reverse (bool) // 反轉
│   ├── duty_val (float32_t) // DEG duty值
│   ├── duty_h (MotorPhaseDuty) // DEG uvw duty
│   │   ├── u (float32_t)
│   │   ├── v (float32_t)
│   │   └── w (float32_t)
│   ├── pi_omega (PI_CTRL)
│   └── pi_current (PI_CTRL)
├── foc_h (MotorFOCParameter)
│   ├── init_cnt (uint32_t) // 無感測啟動計數(尚未)
│   ├── clarke_h (CLARKE) // clarke
│   ├── hall_rad (float32_t) // 目前霍爾相位
│   ├── rotor_rad (float32_t) // 轉子位置
│   ├── rad_itpl (volatile float32_t) // FOC 應補角度 (Angle Interpolation)
│   ├── rad_acc (float32_t) // FOC 角度累積插值 rad_acc += rad_itpl; 過一霍爾中斷後重置
│   ├── park_h (PARK) // park
│   ├── pi_omega (PI_CTRL)
│   ├── pi_Id_h (PI_CTRL)
│   ├── pi_Iq_h (PI_CTRL)
│   ├── magn_rad (float32_t) // 磁場位置
│   ├── ipark_h (IPARK) // ipark
│   ├── svgendq_h (SVGENDQ) // svgendq
│   ├── Vref (float32_t)
│   └── duty_h (MotorPhaseDuty) // FOC duty
│       ├── u (float32_t)
│       ├── v (float32_t)
│       └── w (float32_t)
├── duty_load (MotorPhaseDuty) // PWM load duty
│   ├── u (float32_t)
│   ├── v (float32_t)
│   └── w (float32_t)
└── history (MotorHistoryArray)
    ├── tick (uint8_t)
    ├── id[10] (volatile float32_t)
    └── iq[10] (volatile float32_t)
```