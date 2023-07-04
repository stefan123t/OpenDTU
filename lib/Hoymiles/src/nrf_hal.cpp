#include "nrf_hal.h"

#include "spi_patcher.h"

#define NRF_MAX_TRANSFER_SZ 64

nrf_hal::nrf_hal(int8_t _pin_mosi, int8_t _pin_miso, int8_t _pin_clk, int8_t _pin_cs, int8_t _pin_en) :
    pin_mosi(_pin_mosi),
    pin_miso(_pin_miso),
    pin_clk(_pin_clk),
    pin_cs(_pin_cs),
    pin_en(_pin_en)
{

}

void nrf_hal::patch_spi(spi_host_device_t host_device)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = pin_mosi,
        .miso_io_num = pin_miso,
        .sclk_io_num = pin_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = NRF_MAX_TRANSFER_SZ,
        .flags = 0,
        .intr_flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(host_device, &buscfg, SPI_DMA_DISABLED));

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 1,
        .cs_ena_posttrans = 1,
        .clock_speed_hz = 10000000,
        .input_delay_ns = 0,
        .spics_io_num = pin_cs,
        .flags = 0,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL
    };
    ESP_ERROR_CHECK(spi_bus_add_device(host_device, &devcfg, &spi));
}

void nrf_hal::unpatch_spi(spi_host_device_t host_device)
{
    spi_bus_remove_device(spi);
    spi_bus_free(host_device);

    pinMode(pin_cs, OUTPUT);
    digitalWrite(pin_cs, HIGH);
}

void nrf_hal::request_spi(void)
{
    spi_patcher_inst.request(0, std::bind(&nrf_hal::patch_spi, this, std::placeholders::_1), std::bind(&nrf_hal::unpatch_spi, this, std::placeholders::_1));
}

bool nrf_hal::begin(void)
{
    pinMode(pin_en, OUTPUT);
    digitalWrite(pin_en, LOW);

    delay(10);

    return true;
}

void nrf_hal::end(void)
{

}

void nrf_hal::ce(bool level)
{
    digitalWrite(pin_en, level ? HIGH : LOW);
}

uint8_t nrf_hal::write(uint8_t cmd, const uint8_t* buf, uint8_t len)
{
    request_spi();

    uint8_t data[NRF_MAX_TRANSFER_SZ];
    data[0] = cmd;
    for (uint8_t i = 0; i < len; ++i) {
        data[(size_t)i + 1] = buf[i];
    }

    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = ((size_t)len + 1u) << 3,
        .rxlength = ((size_t)len + 1u) << 3,
        .user = NULL,
        .tx_buffer = data,
        .rx_buffer = data
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));

    //delayMicroseconds(100);

    return data[0]; // status
}

uint8_t nrf_hal::write(uint8_t cmd, const uint8_t* buf, uint8_t data_len, uint8_t blank_len)
{
    request_spi();

    uint8_t data[NRF_MAX_TRANSFER_SZ];
    data[0] = cmd;
    for (uint8_t i = 0; i < data_len; ++i) {
        data[(size_t)i + 1u] = buf[i];
    }
    for (uint8_t i = 0; i < blank_len; ++i) {
        data[(size_t)i + data_len + 1u] = 0;
    }

    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = ((size_t)data_len + blank_len + 1u) << 3,
        .rxlength = ((size_t)data_len + blank_len + 1u) << 3,
        .user = NULL,
        .tx_buffer = data,
        .rx_buffer = data
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));

    //delayMicroseconds(100);
    
    return data[0]; // status
}

uint8_t nrf_hal::read(uint8_t cmd, uint8_t* buf, uint8_t len)
{
    request_spi();

    uint8_t data[NRF_MAX_TRANSFER_SZ];
    data[0] = cmd;
    for (uint8_t i = 0; i < len; ++i) {
        data[(size_t)i + 1u] = 0xff;
    }

    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = ((size_t)len + 1u) << 3,
        .rxlength = ((size_t)len + 1u) << 3,
        .user = NULL,
        .tx_buffer = data,
        .rx_buffer = data
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));

    //delayMicroseconds(100);

    for (uint8_t i = 0; i < len; ++i) {
        buf[i] = data[(size_t)i + 1u];
    }
    return data[0]; // status
}

uint8_t nrf_hal::read(uint8_t cmd, uint8_t* buf, uint8_t data_len, uint8_t blank_len)
{
    request_spi();

    uint8_t data[NRF_MAX_TRANSFER_SZ];
    data[0] = cmd;
    for (uint8_t i = 0; i < data_len; ++i) {
        data[(size_t)i + 1u] = 0xff;
    }
    for (uint8_t i = 0; i < blank_len; ++i) {
        data[(size_t)i + data_len + 1u] = 0xff;
    }

    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = ((size_t)data_len + blank_len + 1u) << 3,
        .rxlength = ((size_t)data_len + blank_len + 1u) << 3,
        .user = NULL,
        .tx_buffer = data,
        .rx_buffer = data
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));

    //delayMicroseconds(100);

    for (uint8_t i = 0; i < data_len; ++i) {
        buf[i] = data[(size_t)i + 1u];
    }
    return data[0]; // status
}
