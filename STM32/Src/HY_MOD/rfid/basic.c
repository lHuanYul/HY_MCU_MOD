#include "HY_MOD/rfid/basic.h"
#ifdef HY_MOD_STM32_RFID

#include "spi.h"

RC522Parametar rfid_h = {
    .const_h = {
        .hspi = &hspi2,
        .SDA_GPIOx = GPIOB,
        .SDA_GPIO_PIN_x = GPIO_PIN_2,
        .IRQ_GPIOx = GPIOB,
        .IRQ_GPIO_PIN_x = GPIO_PIN_1,
        .RST_GPIOx = GPIOA,
        .RST_GPIO_PIN_x = GPIO_PIN_9,
    },
};

#endif