#include "HY_MOD/spi_json/basic.h"
#ifdef HY_MOD_STM32_SPI_JSON

#include "HY_MOD/spi/basic.h"

SpiJsonParametar spi_json_h = {
    .const_h =
    {
        .spi_p = &spi1_h,
        .CS_NSS  = {GPIOD, GPIO_PIN_14},
    },
};

JsonPktPool json_pkt_pool = {0};

static JsonPkt* recv_pkts[JSON_RECV_BUF_CAP];
JsonPktBuf spi_recv_buf = {
    .buf = recv_pkts,
    .cap = JSON_RECV_BUF_CAP,
};

static JsonPkt* trsm_pkts[JSON_TRSM_BUF_CAP];
JsonPktBuf spi_trsm_buf = {
    .buf = trsm_pkts,
    .cap = JSON_TRSM_BUF_CAP,
};

Result spi_json_init(SpiJsonParametar *spij)
{
    spi_init(spij->const_h.spi_p);
    return RESULT_OK(spij);
}

#endif
