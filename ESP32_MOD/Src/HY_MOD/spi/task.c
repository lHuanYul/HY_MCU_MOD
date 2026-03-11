#include "HY_MOD/spi/task.h"
#ifdef HY_MOD_ESP32_SPI

#include "main/mod_cfg.h"

#define SPI_JSON_START_TRCV(spi, len) spi_start_transceive(&(spi)->spi_p, (spi)->tx_pkt->data,(spi)->rx_pkt->data, (len), 5000)
void StartSpiTask(void *argument)
{
    static const char *TAG = "MY_SPI_TASK";
    json_pkt_pool_init(&json_pkt_pool);
    SpiJsonParametar *spi = &spi2_h;
    spi_init(&spi->spi_p);

    const char json_response[] = "{\"RC\":\"SC\"}";
    JsonPkt *tx_normal = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
    json_pkt_set_len(tx_normal, (uint16_t)strlen(json_response));
    memcpy(tx_normal->data, json_response, tx_normal->len);
    RESULT_CHECK_HANDLE(json_pkt_buf_push(&spi_trsm_buf, tx_normal, &json_pkt_pool, 0));

    ESP_LOGI(TAG, "SPI Slave Initialized");
    while (1)
    {
        switch (SPI_JSON_STATE_GET(spi))
        {
            case SPI_JSON_STATE_OK:
            {
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_HEAD);
                break;
            }
            case SPI_JSON_STATE_ERR:
            {
                SPI_JSON_STATE_SET(spi, SPI_JSON_STATE_OK);
                break;
            }
            case SPI_JSON_STATE_HEAD:
            {
                spi->rx_pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
                spi->rx_pkt->data[0] = '\0';
                spi->tx_pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
                spi->tx_pkt->len = sizeof(SPI_LENGTH_H);
                memcpy(spi->tx_pkt->data, SPI_LENGTH_H, spi->tx_pkt->len);
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
                ESP_LOGI(TAG, "Trcv Head Ready: %d", spi->tx_pkt->len);
                RESULT_CHECK_GOTO(SPI_JSON_START_TRCV(spi, spi->tx_pkt->len), head_err);
                ESP_LOGI(TAG, "Trcv Head LEN: %d", spi->spi_p.slave.trans_len);
                ESP_LOG_BUFFER_HEX(TAG, spi->rx_pkt->data, 8);
                uint8_t *data = spi->rx_pkt->data;
                uint16_t payload_len = var_u8_to_u16_be(data + 3);
                if (!(
                    // spi->spi_p.slave.trans_len / U8_BIT_SIZE == sizeof(SPI_LENGTH_H) &&
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
                ESP_LOGE(TAG, "Trcv Head Err");
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
                uint16_t len = spi->rx_pkt->len;;
                if (spi->tx_hold != NULL)
                {
                    json_pkt_pool_free(&json_pkt_pool, spi->tx_pkt);
                    spi->tx_pkt = spi->tx_hold;
                    len = (spi->tx_pkt->len > spi->rx_pkt->len) ? spi->tx_pkt->len : spi->rx_pkt->len;
                }
                ESP_LOGI(TAG, "Trcv Body Ready: %d", len);
                RESULT_CHECK_GOTO(SPI_JSON_START_TRCV(spi, len), body_err);
                ESP_LOGI(TAG, "Trcv Body LEN: %d", spi->spi_p.slave.trans_len);
                ESP_LOG_BUFFER_HEX(TAG, spi->rx_pkt->data, 8);
                if (spi->rx_pkt->len > 0)
                {
                    uint8_t *data = spi->rx_pkt->data;
                    if (!(
                        // spi->spi_p.slave.trans_len / U8_BIT_SIZE == len &&
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
                ESP_LOGE(TAG, "Trcv Body Err");
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
        continue;
    }
    vTaskDelete(NULL);
}

#endif