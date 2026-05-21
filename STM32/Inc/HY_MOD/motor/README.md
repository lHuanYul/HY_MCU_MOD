MotorParameter
├── const_h (const MotorConst) // 常數
│   ├── model (const MotorModelData*) // 指向馬達規格參數 (來自型號定義)
│   │   ├── pole (uint8_t) // 極數
│   │   ├── gear (float32_t) // 減速比
│   │   ├── rated_current (float32_t) // 額定電流
│   │   ├── rl (float32_t) // 定子相電阻
│   │   ├── tau (float32_t) // 電氣時間常數
│   │   ├── ll (float32_t) // 定子相電感 (rl * tau)
│   │   ├── hall_angle_comp (float32_t) // 霍爾訊號與實際電角補償
│   │   ├── deg_spd_Kp (float32_t)
│   │   ├── deg_spd_Ki (float32_t)
│   │   ├── foc_spd_Kp (float32_t)
│   │   └── foc_spd_Ki (float32_t)
│   ├── Hall_GPIO (MotorPhaseGPIOData) // HALL PIN
│   │   ├── u / v / w (GPIOData)
│   │   └── uvw[3] (GPIOData)
│   ├── PWM_htimx (TIM_HandleTypeDef*) // PWM 定時器控制控制代碼
│   ├── PWM_tim_clk (uint32_t*) // PWM 定時器時脈
│   ├── PWM_TIM_CH_x (MotorPhasePwmCH) // PWM 通道配置
│   │   ├── u / v / w (uint32_t)
│   │   └── mid (uint32_t)
│   ├── PWMN_GPIO (MotorPhaseGPIOData) // PWM 互補通道 GPIO
│   │   └── u / v / w (GPIOData)
│   ├── PWMN_GPIO_set (MotorPhaseNPwmGPIO) // PWM 互補通道模式設定
│   │   └── u / v / w (MotorPwmNGpio)
│   │       ├── MODEx / MODEx_0 / MODEx_1 (uint32_t)
│   ├── Hall_htimx (TIM_HandleTypeDef*) // 霍爾計時器控制控制代碼
│   └── Hall_tim_clk (uint32_t*) // 霍爾計時器時脈
├── calcu_h (MotorCalcuConst) // 計算常數
│   ├── pwm_f (float32_t) // PWM 頻率
│   ├── pwm_T (float32_t) // PWM 每個計數週期的時間 (秒/計數)
│   ├── hall_f (float32_t) // 霍爾計時器的實際計數頻率
│   ├── hall_T (float32_t) // 霍爾計時器每個計數週期的時間 (秒/計數)
│   ├── pwm_it_f (float32_t) // PWM 中斷頻率
│   ├── omega_fbk (float32_t) // 霍爾間隔 → 輸出軸轉速 (omega) 轉換常數
│   └── foc_it_angle_itpl (float32_t) // PWM 週期 → 電角度內插轉換常數
├── init_cnt (uint32_t) // 初始化計數器
├── ctrl_h (MotorCtrlParam) // 馬達控制模式 (120度與 FOC 以及細部)
│   ├── ref_ori (MotorCtrl) // 原始控制模式參考
│   └── ref_fix (MotorCtrl) // 修正後控制模式參考
├── rotate_h (MotorRotParam) // 馬達旋轉模式 (滑行與煞車等)
│   ├── ref_ori (MotorRot)
│   └── ref_fix (MotorRot)
├── speed_h (MotorSpdParame) // 轉速參數 (從座往轉子 順時針為負)
│   ├── ref_rpm (float32_t) // 目的轉速 (RPM)
│   ├── ref_omega (float32_t) // 目的角速度 (rad/s)
│   ├── fbk_rpm (float32_t) // 回授轉速 (RPM)
│   ├── fbk_omega (float32_t) // 回授角速度 (rad/s)
│   └── save_stop_omega (float32_t) // 安全停轉角速度阈值
├── tim_tick (uint32_t) // 計時中斷計數
├── adc_h (MotorADC) // 電流 ADC 數據
│   ├── adc_u / adc_v / adc_w (AdcCurrentParameter*) // 各相 ADC 配置結構體指標
│   ├── adc_uvw[3] (AdcCurrentParameter*)
│   ├── u / v / w (float32_t) // 各相電流標么值 (Per-Unit)
│   ├── uvw[3] (float32_t)
│   └── total (float32_t) // 三相電流總和 (理論上應接近 0)
├── hall_h (MotorHallParam) // 霍爾感測器狀態
│   ├── time_hist_head (volatile uint8_t) // 霍爾跳變間隔歷史緩衝區標頭 ID
│   ├── time_hist_len (volatile uint8_t) // 霍爾跳變間隔歷史長度
│   ├── time_hist[MOTOR_SPD_CNT] (uint32_t) // 霍爾跳變間隔時間歷史記錄
│   ├── current (volatile uint8_t) // 目前霍爾狀態/相位
│   ├── last (uint8_t) // 上次霍爾狀態/相位
│   ├── wrong (volatile uint8_t) // 錯誤狀態標記
│   ├── virtual (volatile uint8_t) // 虛擬霍爾相位 (用於自動旋轉測試)
│   └── stop_tick (uint32_t) // 停轉判定計時器
├── sless_h (MotorSLessParam) // 無感測器控制參數 (保留擴充用)
├── deg_h (MotorDEGParameter) // 120度方波控制參數
│   ├── reverse (bool) // 反轉標記
│   ├── duty_val (float32_t) // DEG 模式總占空比值
│   ├── duty_h (MotorPhaseDuty) // DEG 模式三相個別占空比
│   │   └── u / v / w / uvw[3] (float32_t)
│   ├── pi_omega (PI_CTRL) // 速度環 PI 控制器
│   └── pi_current (PI_CTRL) // 電流環 PI 控制器
├── foc_h (MotorFOCParameter) // FOC 核心控制參數
│   ├── init_cnt (uint32_t) // FOC 初始化計數
│   ├── clarke_h (CLARKE) // Clarke 轉換結構體
│   ├── hall_rad (float32_t) // 目前霍爾位置對應的弧度
│   ├── rotor_rad (float32_t) // 轉子實際電角度弧度
│   ├── rad_itpl (volatile float32_t) // FOC 角度內插增量 (Angle Interpolation)
│   ├── rad_acc (float32_t) // 內插角度累積值 (過下一個霍爾中斷後重置)
│   ├── park_h (PARK) // Park 轉換結構體
│   ├── pi_omega (PI_CTRL) // FOC 速度環 PI 控制器
│   ├── pi_Id_h (PI_CTRL) // D 軸電流 PI 控制器
│   ├── pi_Iq_h (PI_CTRL) // Q 軸電流 PI 控制器
│   ├── magn_rad (float32_t) // 逆變器磁場電角度位置
│   ├── ipark_h (IPARK) // 反 Park 轉換結構體
│   ├── svgendq_h (SVGENDQ) // SVPWM 核心運算結構體
│   ├── Vref_s (float32_t) // 電壓向量基準值 (SQRT3 * Vref / Vbus)
│   └── duty_h (MotorPhaseDuty) // FOC 最終輸出的三相占空比
│       └── u / v / w / uvw[3] (float32_t)
├── duty_load (MotorPhaseDuty) // 最終載入定時器暫存器的 PWM 占空比數據
│   └── u / v / w / uvw[3] (float32_t)
├── dbg_h (MotorDbg) // 偵錯用變數
│   ├── hall_rad[8] (float32_t) // 儲存不同霍爾狀態對應的校準弧度
│   └── hall_last (uint8_t) // 上次偵錯記錄的霍爾狀態
└── history (MotorHistoryArray) // 歷史數據取樣緩衝區 (常用於 Scope 觀察)
    ├── tick (uint8_t) // 取樣計數
    ├── id[10] (volatile float32_t) // D 軸電流歷史緩衝區
    └── iq[10] (volatile float32_t) // Q 軸電流歷史緩衝區