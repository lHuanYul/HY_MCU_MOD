#include "HY_MOD/spi/main.h"
#ifdef HY_MOD_ESP32_SPI

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/spi/callback.h"

#include "esp_heap_caps.h"

static const char *TAG = "MY_SPI_MAIN";

#define SPI_MASTER_ASK  "$READ  "
#define SPI_LENGTH_H    "$L:HL  "

// --- 緩衝區設定 ---
// 設定一個足夠大的 Buffer 來放 JSON (例如 256 bytes)
// 如果您的 JSON 很長，請加大這裡，但必須是 4 的倍數

void spi_init(SpiParametar *spi)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = spi->const_h.MOSI,
        .miso_io_num = spi->const_h.MISO,
        .sclk_io_num = spi->const_h.SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .intr_flags = ESP_INTR_FLAG_IRAM,
    };
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = spi->const_h.NSS,
        .queue_size = 3,    // 交易隊列大小
        .flags = 0,
        .post_trans_cb = spi_done_cb,
        // .post_setup_cb = my_post_setup_cb, // 如果需要 Callback 可設
    };
    esp_err_t ret = spi_slave_initialize(spi->const_h.SPIx_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
}

static Result spi_trcv(SpiParametar *spi, uint32_t timeout_ms)
{
    spi->txrx = xTaskGetCurrentTaskHandle();
    ESP_ERROR_CHECK(spi_slave_queue_trans(spi->const_h.SPIx_HOST, &spi->slave, portMAX_DELAY));
    // if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms)) == 0)
    //     return RESULT_ERROR(RES_ERR_BUSY);
    spi_slave_transaction_t *spi_o;
    ESP_ERROR_CHECK(spi_slave_get_trans_result(spi->const_h.SPIx_HOST, &spi_o, portMAX_DELAY));
    if (spi_o != &spi->slave)
        return RESULT_ERROR(RES_ERR_FAIL);
    return RESULT_OK(NULL);
}

Result spi_start_transceive(SpiParametar *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len, uint32_t timeout_ms)
{
    memset(&spi->slave, 0, sizeof(spi_slave_transaction_t));
    spi->slave.tx_buffer = tx_buf;
    spi->slave.rx_buffer = rx_buf;
    spi->slave.length = len * U8_BIT_SIZE;
    spi->slave.user = (void*)spi;
    return spi_trcv(spi, timeout_ms);
    // spi->rx_buf_len = spi->slave.trans_len / U8_BIT_SIZE;
}

Result spi_start_receive(SpiParametar *spi, uint16_t byte_len, uint8_t *rx_buf, uint32_t timeout_ms)
{
    memset(&spi->slave, 0, sizeof(spi_slave_transaction_t));
    spi->slave.rx_buffer = rx_buf;
    spi->slave.length = byte_len * U8_BIT_SIZE;
    spi->slave.user = (void*)spi;
    return spi_trcv(spi, timeout_ms);
}

Result spi_start_transmit(SpiParametar *spi, uint16_t byte_len, uint8_t *tx_buf, uint32_t timeout_ms)
{
    memset(&spi->slave, 0, sizeof(spi_slave_transaction_t));
    spi->slave.length = byte_len * U8_BIT_SIZE;
    spi->slave.tx_buffer = tx_buf;
    spi->slave.user = (void*)spi;
    return spi_trcv(spi, timeout_ms);
}

#endif