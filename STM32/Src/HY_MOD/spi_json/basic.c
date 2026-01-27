#include "HY_MOD/spi_json/basic.h"
#ifdef HY_MOD_STM32_SPI_JSON

#include "HY_MOD/spi/basic.h"

Result spi_json_init(SpiJsonParametar *spi)
{
    spi_init(&spi->spi_p);
    return RESULT_OK(NULL);
}

#endif