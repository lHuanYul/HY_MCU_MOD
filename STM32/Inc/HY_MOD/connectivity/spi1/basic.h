#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/main/fn_state.h"

#define SPI_MASTER_ASK "$READ  "
#define SPI_LENGTH_H   "$L:HL  "

typedef struct JsonPkt
{
    uint8_t     data[JSON_PKT_LEN + 1];
    uint16_t    len;
    struct JsonPkt *next;
} JsonPkt;
Result json_pkt_get_num(JsonPkt *pkt, char *id, uint64_t *container);
Result json_pkt_get_char(JsonPkt *pkt, char *id, char *container, uint16_t container_cap);
Result json_pkt_set_len(JsonPkt *pkt, uint16_t len);

typedef struct JsonPktPool
{
    JsonPkt     pkt[JSON_PKT_POOL_CAP];
    JsonPkt*    head;
    uint8_t     remain;
} JsonPktPool;
extern JsonPktPool json_pkt_pool;
void json_pkt_pool_init(void);
Result json_pkt_pool_alloc(void);
void json_pkt_pool_free(JsonPkt *pkt);

typedef struct JsonPktBuf
{
    JsonPkt**   buf;
    uint8_t     head;
    uint8_t     len;
    uint8_t     cap;
} JsonPktBuf;
Result json_pkt_buf_push(JsonPktBuf* self, JsonPkt *pkt, uint8_t drop);
Result json_pkt_buf_get(JsonPktBuf* self);
Result json_pkt_buf_pop(JsonPktBuf* self);

typedef struct SpiConst
{
    SPI_HandleTypeDef *hspix;
    GPIOData MISO;
    GPIOData MOSI;
    GPIOData SCK;
    // CS
    GPIOData NSS;
} SpiConst;

typedef enum SpiState
{
    SPI_STATE_FINISH,
    SPI_STATE_ERROR,
    SPI_STATE_RECV_HEADER,
    SPI_STATE_RECV_BODY,
    SPI_STATE_TRSM_HEADER,
    SPI_STATE_TRSM_BODY,
} SpiState;

typedef struct SpiParametar
{
    const SpiConst const_h;
    SpiState state;
    osSemaphoreId_t rx_handle;
    const osSemaphoreAttr_t rx_handle_attr;
    uint16_t rx_buf_len;
    uint8_t *rx_buf;
    JsonPktBuf rx_pkt_buf;
    osSemaphoreId_t tx_handle;
    const osSemaphoreAttr_t tx_handle_attr;
    uint16_t tx_buf_len;
    uint8_t *tx_buf;
    JsonPktBuf tx_pkt_buf;
} SpiParametar;

extern SpiParametar spi1_h;

#endif