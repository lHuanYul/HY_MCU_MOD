#include "HY_MOD/spi_json/main.h"
#ifdef HY_MOD_STM32_SPI

#include "main/mod_cfg.h"
#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/packet/json.h"

uint32_t spi_test = 0;
#define SPI1_TASK_SMAL_MS      5
#define SPI1_TASK_NEXT_MS   1000
void StartSpi1Task(void *argument)
{
    const uint32_t osPeriod_next = pdMS_TO_TICKS(SPI1_TASK_NEXT_MS);
    uint32_t next_wake = osKernelGetTickCount();
    json_pkt_pool_init(&json_pkt_pool);
    SpiParametar *spi = &spi1_h;
    spi_init(spi);

    const char json_response[] = "{\"TR\":\"SC\"}";
    JsonPkt *tx_normal = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
    json_pkt_set_len(tx_normal, (uint16_t)strlen(json_response));
    memcpy(tx_normal->data, json_response, tx_normal->len);
    RESULT_CHECK_HANDLE(json_pkt_buf_push(&spi_trsm_buf, tx_normal, &json_pkt_pool, 0));

    next_wake += osPeriod_next * 5;
    osDelayUntil(next_wake);
    JsonPkt *tx_pkt = NULL;
    for(;;)
    {
        switch (spi1_h.state)
        {
            case SPI_STATE_FINISH:
            {
                osDelayUntil(next_wake);
                next_wake += osPeriod_next;
                spi->tx_buf_len = sizeof(SPI_MASTER_ASK);
                memcpy(spi->tx_buf, SPI_MASTER_ASK, sizeof(SPI_MASTER_ASK));
                RESULT_CHECK_GOTO(spi_start_transmit(spi), error);
                spi_test++;
                spi->state = SPI_STATE_RECV_HEADER;
                break;
            }
            case SPI_STATE_ERROR:
            {
                next_wake += osPeriod_next * 3;
                osDelayUntil(next_wake);
                next_wake += osPeriod_next;
                spi1_h.state = SPI_STATE_FINISH;
                break;
            }
            case SPI_STATE_RECV_HEADER:
            {
                spi->rx_buf_len = sizeof(SPI_LENGTH_H);
                RESULT_CHECK_GOTO(spi_start_receive(spi), error);
                if (memcmp(spi->rx_buf, SPI_SLAVE_EMP, sizeof(SPI_SLAVE_EMP)) == 0)
                {
                    spi1_h.state = SPI_STATE_TRSM_HEADER;
                }
                else if (
                    spi->rx_buf[0] == '$' &&
                    spi->rx_buf[1] == 'L' &&
                    spi->rx_buf[2] == ':' &&
                    spi->rx_buf[spi->rx_buf_len - 1] == '\0'
                ) {
                    spi->state = SPI_STATE_RECV_BODY;
                    uint16_t payload_len = var_u8_to_u16_be(spi->rx_buf + 3);
                    if (payload_len > JSON_PKT_LEN)
                    {
                        spi->state = SPI_STATE_ERROR;
                        break;
                    }
                    spi->rx_buf_len = payload_len;
                }
                else goto error;
                break;
            }
            case SPI_STATE_RECV_BODY:
            {
                RESULT_CHECK_GOTO(spi_start_receive(spi), error);
                if (
                    spi->rx_buf_len > 0      &&
                    spi->rx_buf[0] == '{'    &&
                    spi->rx_buf[spi->rx_buf_len - 1] == '}'
                ) {
                    spi->state = SPI_STATE_TRSM_HEADER;
                    spi->rx_buf[spi->rx_buf_len] = '\0';
                    JsonPkt *pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
                    RESULT_CHECK_HANDLE(json_pkt_set_len(pkt, spi->rx_buf_len + 1));
                    memcpy(pkt->data, spi->rx_buf, spi->rx_buf_len + 1);
                    json_pkt_buf_push(&spi_recv_buf, pkt, &json_pkt_pool, 1);
                }
                else goto error;
                break;
            }
            case SPI_STATE_TRSM_HEADER:
            {
                tx_pkt = RESULT_UNWRAP_GOTO(json_pkt_buf_get(&spi_trsm_buf), tx_skip);
                spi->tx_buf_len = sizeof(SPI_LENGTH_H);
                memcpy(spi->tx_buf, SPI_LENGTH_H, spi->tx_buf_len);
                var_u16_to_u8_be(tx_pkt->len, (uint8_t *)(spi->tx_buf + 3));
                RESULT_CHECK_GOTO(spi_start_transmit(spi), tx_skip);
                spi->state = SPI_STATE_TRSM_BODY;
                break;
            }
            case SPI_STATE_TRSM_BODY:
            {
                if (tx_pkt == NULL) goto error;
                spi->tx_buf_len = tx_pkt->len;
                memcpy(spi->tx_buf, tx_pkt->data, spi->tx_buf_len);
                RESULT_CHECK_GOTO(spi_start_transmit(spi), tx_skip);
                spi->state = SPI_STATE_FINISH;
                tx_pkt = NULL;
                JsonPkt *pkt = RESULT_UNWRAP_HANDLE(json_pkt_buf_pop(&spi_trsm_buf));
                json_pkt_pool_free(&json_pkt_pool, pkt);
                break;
            }
        }
        osDelay(SPI1_TASK_SMAL_MS);
        continue;
tx_skip:
        tx_pkt = NULL;
        spi->state = SPI_STATE_FINISH;
        continue;
error:
        spi->state = SPI_STATE_ERROR;
    }

    StopTask();
}

#endif