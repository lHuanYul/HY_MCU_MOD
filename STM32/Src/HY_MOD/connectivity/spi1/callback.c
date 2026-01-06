#include "HY_MOD/connectivity/spi1/callback.h"
#ifdef HY_MOD_STM32_SPI
#include "HY_MOD/main/variable_cal.h"

void spi1_rx_callback(void)
{
    SpiParametar *spi = &spi1_h;
    GPIO_WRITE(spi->const_h.NSS, 1);
    #if defined(STM32F7) || defined(STM32H7)
        SCB_InvalidateDCache_by_Addr((uint32_t*)spi->rx_buf, JSON_PKT_LEN+32);
    #endif
    
    switch (spi->state)
    {
        case SPI_STATE_RECV_HEADER:
        {
            if (
                spi->rx_buf[0] == '$' &&
                spi->rx_buf[1] == 'L' &&
                spi->rx_buf[2] == ':' &&
                spi->rx_buf[spi->rx_buf_len - 1] == '\0'
            ) {
                spi->state = SPI_STATE_RECV_BODY;
                uint16_t payload_len = var_u8_to_u16_be(spi->rx_buf + 3);
                if (payload_len > JSON_PKT_LEN) goto error;
                spi->rx_buf_len = payload_len;
                osSemaphoreRelease(spi->rx_handle);
                return;
            }
            break;
        }
        case SPI_STATE_RECV_BODY:
        {
            if (
                spi->rx_buf_len > 0      &&
                spi->rx_buf[0] == '{'    &&
                spi->rx_buf[spi->rx_buf_len - 1] == '}'
            ) {
                spi->state = SPI_STATE_FINISH;
                spi->rx_buf[spi->rx_buf_len] = '\0';
                JsonPkt *pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc());
                json_pkt_set_len(pkt, spi->rx_buf_len);
                memcpy(pkt->data, spi->rx_buf, spi->rx_buf_len + 1);
                json_pkt_buf_push(&spi->rx_pkt_buf, pkt, 1);
                osSemaphoreRelease(spi->rx_handle);
                return;
            }
            break;
        }
        default: break;
    }
error:
    spi->state = SPI_STATE_ERROR;
    osSemaphoreRelease(spi->rx_handle);
}

void spi1_tx_callback(void)
{
    SpiParametar *spi = &spi1_h;
    switch (spi->state)
    {
        case SPI_STATE_FINISH:
        {
            spi->state = SPI_STATE_RECV_HEADER;
            osSemaphoreRelease(spi->tx_handle);
            return;
        }
        case SPI_STATE_TRSM_HEADER:
        {
            spi->state = SPI_STATE_TRSM_BODY;
            osSemaphoreRelease(spi->tx_handle);
            return;
        }
        case SPI_STATE_TRSM_BODY:
        {
            
            spi->state = SPI_STATE_FINISH;
            // ...
            osSemaphoreRelease(spi->tx_handle);
            return;
        }
        default: break;
    }
error:
    spi->state = SPI_STATE_ERROR;
    osSemaphoreRelease(spi->tx_handle);
}

#endif