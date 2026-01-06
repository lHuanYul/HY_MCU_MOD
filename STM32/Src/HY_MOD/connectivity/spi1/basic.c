#include "HY_MOD/connectivity/spi1/basic.h"
#ifdef HY_MOD_STM32_SPI
#include "CoreJSON/core_json.h"
#include "spi.h"

__attribute__((aligned(32))) static uint8_t rx_buf[JSON_PKT_LEN + 32];
static JsonPkt* recv_pkts[JSON_RECV_BUF_CAP];

__attribute__((aligned(32))) static uint8_t tx_buf[JSON_PKT_LEN + 32];
static JsonPkt* trsm_pkts[JSON_TRSM_BUF_CAP];

SpiParametar spi1_h = {
    .const_h = {
        .hspix = &hspi1,
        .MISO = {GPIOA, GPIO_PIN_6},
        .MOSI = {GPIOA, GPIO_PIN_7},
        .SCK = {GPIOD, GPIO_PIN_8},
        .NSS = {GPIOD, GPIO_PIN_9},
    },
    .rx_handle_attr = {
        .name = "spiRxSem"
    },
    .rx_buf = rx_buf,
    .rx_pkt_buf = {
        .buf = recv_pkts,
        .cap = JSON_RECV_BUF_CAP,
    },
    .tx_handle_attr = {
        .name = "spiTxSem"
    },
    .tx_buf = tx_buf,
    .tx_pkt_buf = {
        .buf = trsm_pkts,
        .cap = JSON_TRSM_BUF_CAP,
    },
};

JsonPktPool json_pkt_pool;

Result json_pkt_get_num(JsonPkt *pkt, char *id, uint64_t *container)
{
    char *value;
    size_t valueLength;
    JSONTypes_t type;
    JSONStatus_t status;

    status = JSON_SearchT(
        (char *)pkt->data, pkt->len,
        id, strlen(id),
        &value, &valueLength, &type
    );

    if (status != JSONSuccess || type != JSONNumber)
        return RESULT_ERROR(RES_ERR_NOT_FOUND);
    if (valueLength > 64) return RESULT_ERROR(RES_ERR_OVERFLOW);

    char temp[65];
    memcpy(temp, value, valueLength);
    temp[valueLength] = '\0';
    *container = (uint64_t)strtoull(temp, NULL, 10);

    return RESULT_OK(NULL);
}

Result json_pkt_get_char(
    JsonPkt *pkt, char *id,
    char *container, uint16_t container_cap
) {
    char *value;
    size_t valueLength;
    JSONTypes_t type;
    JSONStatus_t status;

    status = JSON_SearchT(
        (char *)pkt->data, pkt->len,
        id, strlen(id),
        &value, &valueLength, &type
    );
    
    if (status != JSONSuccess || type != JSONString)
        return RESULT_ERROR(RES_ERR_NOT_FOUND);
    if (valueLength >= container_cap) return RESULT_ERROR(RES_ERR_OVERFLOW);
    if (valueLength < 2) return RESULT_ERROR(RES_ERR_INVALID);

    uint16_t contentLen = (uint16_t)valueLength - 2;
    memcpy(container, value + 1, contentLen);
    container[contentLen] = '\0';

    return RESULT_OK(NULL);
}

Result json_pkt_set_len(JsonPkt *pkt, uint16_t len)
{
    if (len > JSON_PKT_LEN) return RESULT_ERROR(RES_ERR_FULL);
    pkt->len = len;
    return RESULT_OK(pkt);
}

void json_pkt_pool_init(void)
{
    json_pkt_pool.head = NULL;
    for (uint8_t i = 0; i < JSON_PKT_POOL_CAP; i++) {
        json_pkt_pool.pkt[i].next = json_pkt_pool.head;
        json_pkt_pool.head = &json_pkt_pool.pkt[i];
    }
    json_pkt_pool.remain = JSON_PKT_POOL_CAP;
}

Result json_pkt_pool_alloc(void)
{
    if (
        json_pkt_pool.head == NULL ||
        json_pkt_pool.remain == 0
    ) while(1);
    JsonPkt *pkt = json_pkt_pool.head;
    json_pkt_pool.head = pkt->next;
    pkt->next = NULL;
    json_pkt_pool.remain--;
    return RESULT_OK(pkt);
}

void json_pkt_pool_free(JsonPkt *pkt)
{
    if (json_pkt_pool.remain >= JSON_PKT_POOL_CAP) while(1);
    memset(pkt->data, 0, sizeof(pkt->data));
    pkt->len = 0;
    pkt->next = json_pkt_pool.head;
    json_pkt_pool.head = pkt;
    json_pkt_pool.remain++;
}

// 0:error 1:drop_first 2:drop_current
Result json_pkt_buf_push(JsonPktBuf* self, JsonPkt *pkt, uint8_t drop)
{
    if (self->len >= self->cap)
    {
        switch (drop)
        {
            case 1:
            {
                JsonPkt *pkt_d = RESULT_UNWRAP_HANDLE(json_pkt_buf_pop(self));
                json_pkt_pool_free(pkt_d);
                break;
            }
            case 2:
            {
                json_pkt_pool_free(pkt);
                return RESULT_OK(self);
            }
            default: return RESULT_ERROR(RES_ERR_OVERFLOW);
        }
    }
    uint8_t tail = (self->head + self->len) % self->cap;
    self->buf[tail] = pkt;
    self->len++;
    return RESULT_OK(self);
}

Result json_pkt_buf_get(JsonPktBuf* self)
{
    if (self->len == 0) return RESULT_ERROR(RES_ERR_EMPTY);
    JsonPkt *pkt = self->buf[self->head];
    return RESULT_OK(pkt);
}

Result json_pkt_buf_pop(JsonPktBuf* self)
{
    JsonPkt *pkt = RESULT_UNWRAP_RET_RES(json_pkt_buf_get(self));
    if (--self->len == 0) self->head = 0;
    else self->head = (self->head + 1) % self->cap;
    return RESULT_OK(pkt);
}

#endif