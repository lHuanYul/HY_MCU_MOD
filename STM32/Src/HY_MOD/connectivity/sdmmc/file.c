#include "HY_MOD/connectivity/sdmmc/file.h"
#ifdef HY_MOD_STM32_SDMMC

FILE_OWN file_test = {
    .const_h = {
        .name = "test.txt",
        .byte = sizeof(uint32_t),
    }
};

#endif