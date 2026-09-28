#include "HY_MOD/main/fn_state.h"

ResultErrorType last_error;

#ifdef PRINCIPAL_PROGRAM
#include "HY_MOD/vehicle/basic.h"

Result_h error_state;

void timeout_error(uint32_t start_time, Result *error_parameter) {
    osDelay(10);
    if (!runtime_switch.timeout) return;

    if (HAL_GetTick() - start_time > ERROR_TIMEOUT_TIME_LIMIT) {
        *error_parameter = RESULT_ERROR(RESULT_ERROR_TIMEOUT);
        vehicle_ensure_stop();
        while (true) osDelay(10);
    }
}

#endif

#ifdef AGV_ESP32_DEVICE

void Error_Handler(void)
{
    while (1)
    {
    }
}

#endif
