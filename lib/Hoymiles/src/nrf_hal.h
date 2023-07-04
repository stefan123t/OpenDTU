#pragma once

#include <RF24_hal.h>
#include <driver/spi_master.h>

class nrf_hal : public RF24_hal
{
public:
    nrf_hal(int8_t pin_mosi, int8_t pin_miso, int8_t pin_clk, int8_t pin_cs, int8_t pin_en);

    void patch_spi(spi_host_device_t host_device);
    void unpatch_spi(spi_host_device_t host_device);
    void request_spi(void);

    bool begin(void) override;
    void end(void) override;

    void ce(bool level) override;
    uint8_t write(uint8_t cmd, const uint8_t* buf, uint8_t len) override;
    uint8_t write(uint8_t cmd, const uint8_t* buf, uint8_t data_len, uint8_t blank_len) override;
    uint8_t read(uint8_t cmd, uint8_t* buf, uint8_t len) override;
    uint8_t read(uint8_t cmd, uint8_t* buf, uint8_t data_len, uint8_t blank_len) override;

private:
    const int8_t pin_mosi;
    const int8_t pin_miso;
    const int8_t pin_clk;
    const int8_t pin_cs;
    const int8_t pin_en;
    
    spi_device_handle_t spi;
    Print* print;
};
