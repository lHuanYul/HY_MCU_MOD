#include "HY_MOD/connectivity/spi1/main.h"
#ifdef HY_MOD_STM32_SPI
#include "HY_MOD/main/variable_cal.h"
#include "spi.h"

static Result wait_it(SpiParametar *spi, osSemaphoreId_t tag, uint32_t time_out)
{
    if (osSemaphoreAcquire(tag, time_out) != osOK)
    {
        HAL_SPI_DMAStop(spi->const_h.hspix);
        GPIO_WRITE(spi->const_h.NSS, 1);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    return RESULT_OK(NULL);
}

static Result start_receive(SpiParametar *spi)
{
    GPIO_WRITE(spi->const_h.NSS, 0);
    for(volatile int i = 0; i < 800; i++) __NOP();
    if (HAL_SPI_Receive_DMA(spi->const_h.hspix, spi->rx_buf, spi->rx_buf_len) != HAL_OK)
    {
        GPIO_WRITE(spi->const_h.NSS, 1);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    wait_it(spi, spi->rx_handle, 100);
    return RESULT_OK(NULL);
}

static Result start_transmit(SpiParametar *spi)
{
    GPIO_WRITE(spi->const_h.NSS, 0);
    for(volatile int i = 0; i < 800; i++) __NOP();
    if (HAL_SPI_Transmit_DMA(spi->const_h.hspix, spi->tx_buf, spi->tx_buf_len) != HAL_OK)
    {
        GPIO_WRITE(spi->const_h.NSS, 1);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    wait_it(spi, spi->tx_handle,100);
    while (__HAL_SPI_GET_FLAG(spi->const_h.hspix, SPI_FLAG_BSY));
    GPIO_WRITE(spi->const_h.NSS, 1);
    return RESULT_OK(NULL);
}

uint8_t spi_test = 0;
void StartSpi1Task(void *argument)
{
    json_pkt_pool_init();
    SpiParametar *spi = &spi1_h;
    spi->rx_handle = osSemaphoreNew(1, 0, &spi->rx_handle_attr);
    spi->tx_handle = osSemaphoreNew(1, 0, &spi->tx_handle_attr);

    JsonPkt *my_tx = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc());
    json_pkt_set_len(my_tx, sizeof(SPI_MASTER_ASK));
    memcpy(my_tx->data, SPI_MASTER_ASK, my_tx->len);

    const char json_response[] = "{\"TR\":\"SC\"}";

    osDelay(5000);
    for(;;)
    {
        switch (spi1_h.state)
        {
            case SPI_STATE_ERROR:
            {
                while(1) osDelay(1000);
                spi1_h.state = SPI_STATE_FINISH; 
                goto next_loop;
            }
            case SPI_STATE_FINISH:
            {
                if (spi_test) goto next_loop;
                spi_test++;
                spi->tx_buf_len = my_tx->len;
                memcpy(spi->tx_buf, my_tx->data, my_tx->len);
                RESULT_CHECK_GOTO(start_transmit(spi), next_loop);
                osDelay(100);
                break;
            }
            case SPI_STATE_RECV_HEADER:
            {
                spi->rx_buf_len = sizeof(SPI_LENGTH_H);
                RESULT_CHECK_GOTO(start_receive(spi), next_loop);
                if (spi1_h.state != SPI_STATE_RECV_BODY)
                {
                    goto next_loop;
                }
                osDelay(100);
                break;
            }
            case SPI_STATE_RECV_BODY:
            {
                RESULT_CHECK_GOTO(start_receive(spi), next_loop);
                if (spi1_h.state != SPI_STATE_FINISH)
                {
                    goto next_loop;
                }
                osDelay(100);
                break;
            }
            case SPI_STATE_TRSM_HEADER:
            {
                spi->tx_buf_len = sizeof(SPI_LENGTH_H);
                memcpy(spi->tx_buf, SPI_LENGTH_H, spi->tx_buf_len);
                var_u16_to_u8_be((uint16_t)strlen(json_response), (uint8_t *)(spi->tx_buf + 3));
                RESULT_CHECK_GOTO(start_transmit(spi), next_loop);
                if (spi1_h.state != SPI_STATE_TRSM_BODY)
                {
                    goto next_loop;
                }
                osDelay(100);
                break;
            }
            case SPI_STATE_TRSM_BODY:
            {
                spi->tx_buf_len = (uint16_t)strlen((char*)json_response);
                memcpy(spi->tx_buf, json_response, spi->tx_buf_len);
                RESULT_CHECK_GOTO(start_transmit(spi), next_loop);
                if (spi1_h.state != SPI_STATE_FINISH)
                {
                    goto next_loop;
                }
                osDelay(100);
                break;
            }
        }
        continue;
next_loop:
        osDelay(1000);
    }

    json_pkt_pool_free(my_tx);
}

#endif