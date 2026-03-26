#include "HY_MOD/spi_json/main.h"
#ifdef HY_MOD_STM32_SPI_JSON

#include "main/main.h"
#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/packet/json.h"

#define SPI1_TASK_SMAL_MS      5
#define SPI1_TASK_NEXT_MS   1000
#define SPI_JSON_START_TRCV(spi, len) spi_start_transceive(&(spi)->spi_p,(spi)->tx_pkt->data,(spi)->rx_pkt->data, (len))
void StartSpi1Task(void *argument)
{
    const uint32_t osPeriod_next = pdMS_TO_TICKS(SPI1_TASK_NEXT_MS);
    uint32_t next_wake = osKernelGetTickCount();
    json_pkt_pool_init(&json_pkt_pool);
    SpiJsonParametar *spi = &spi1_h;
    spi_json_init(spi);

    const char json_response[] = "{\"TR\":\"SC\"}";
    JsonPkt *tx_normal = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
    json_pkt_set_len(tx_normal, (uint16_t)strlen(json_response));
    memcpy(tx_normal->data, json_response, tx_normal->len);
    RESULT_CHECK_HANDLE(json_pkt_buf_push(&spi_trsm_buf, tx_normal, &json_pkt_pool, 0));

    next_wake += osPeriod_next;
    osDelayUntil(next_wake);
    for(;;)
    {
        switch (SPI_JSON_STATE_GET(spi))
        {
            case SPI_JSON_STATE_OK:
            {
                osDelayUntil(next_wake);
                next_wake += osPeriod_next;
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_HEAD);
                break;
            }
            case SPI_JSON_STATE_ERR:
            {
                next_wake += osPeriod_next * 3;
                osDelayUntil(next_wake);
                next_wake += osPeriod_next;
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_OK);
                break;
            }
            case SPI_JSON_STATE_HEAD:
            {
                spi->rx_pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
                spi->rx_pkt->data[0] = '\0';
                spi->tx_pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
                spi->tx_pkt->len = sizeof(SPI_LENGTH_H);
                memcpy(spi->tx_pkt->data, SPI_LENGTH_H, sizeof(SPI_LENGTH_H));
                Result res = json_pkt_buf_get(&spi_trsm_buf);
                if (RESULT_CHECK_RAW(res))
                {
                    spi->tx_hold = NULL;
                    var_u16_to_u8_be(0, (spi->tx_pkt->data + 3));
                }
                else
                {
                    spi->tx_hold = res.result.success.obj;
                    var_u16_to_u8_be(spi->tx_hold->len, (spi->tx_pkt->data + 3));
                }
                RESULT_CHECK_GOTO(SPI_JSON_START_TRCV(spi, sizeof(SPI_LENGTH_H)), head_err);

                uint8_t *data = spi->rx_pkt->data;
                uint16_t payload_len = var_u8_to_u16_be(data + 3);
                if (!(
                    data[0] == '$' && data[1] == 'L' &&
                    data[2] == ':' && data[sizeof(SPI_LENGTH_H) - 1] == '\0'
                )) goto head_err;
                if (payload_len == 0 && spi->tx_hold == NULL)
                {
                    SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_OK);
                    goto head_none;
                }
                else if (payload_len <= JSON_PKT_LEN)
                {
                    SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_BODY);
                    spi->rx_pkt->len = payload_len;
                }
                else goto head_err;
                break;
head_err:
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_ERR);
head_none:
                json_pkt_pool_free(&json_pkt_pool, spi->rx_pkt);
                spi->rx_pkt = NULL;
                json_pkt_pool_free(&json_pkt_pool, spi->tx_pkt);
                spi->tx_pkt = NULL;
                break;
            }
            case SPI_JSON_STATE_BODY:
            {
                osDelay(SPI1_TASK_SMAL_MS);
                uint16_t len = spi->rx_pkt->len;;
                if (spi->tx_hold != NULL)
                {
                    json_pkt_pool_free(&json_pkt_pool, spi->tx_pkt);
                    spi->tx_pkt = spi->tx_hold;
                    len = (spi->tx_pkt->len > spi->rx_pkt->len) ? spi->tx_pkt->len : spi->rx_pkt->len;
                }
                RESULT_CHECK_GOTO(SPI_JSON_START_TRCV(spi, len), body_err);
                
                if (spi->rx_pkt->len > 0)
                {
                    uint8_t *data = spi->rx_pkt->data;
                    if (!(
                        data[0] == '{'          &&
                        data[spi->rx_pkt->len - 1] == '}'
                    )) goto body_err;
                    data[spi->rx_pkt->len] = '\0';
                    json_pkt_buf_push(&spi_recv_buf, spi->rx_pkt, &json_pkt_pool, 1);
                }
                else
                {
                    json_pkt_pool_free(&json_pkt_pool, spi->rx_pkt);
                }
                spi->rx_pkt = NULL;
                if (spi->tx_hold != NULL)
                {
                    JsonPkt *pkt = RESULT_UNWRAP_HANDLE(json_pkt_buf_pop(&spi_trsm_buf));
                    if (pkt != spi->tx_pkt) while (1);
                }
                json_pkt_pool_free(&json_pkt_pool, spi->tx_pkt);
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_OK);
                break;
body_err:
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_ERR);
                json_pkt_pool_free(&json_pkt_pool, spi->rx_pkt);
                spi->rx_pkt = NULL;
                if (spi->tx_hold == NULL)
                {
                    json_pkt_pool_free(&json_pkt_pool, spi->tx_pkt);
                    spi->tx_pkt = NULL;
                }
                else spi->tx_pkt = NULL;
                break;
            }
        }
    }

    StopTask();
}

#endif