#pragma once

#include <driver/spi_master.h>

#ifdef __cplusplus

#include <functional>

class spi_patcher
{
public:
    spi_patcher(spi_host_device_t host_device);
    void request(int8_t id, std::function<void(spi_host_device_t)> patch, std::function<void(spi_host_device_t)> unpatch);

private:
    spi_host_device_t m_host_device;
    int8_t m_id;
    std::optional<std::function<void(spi_host_device_t)>> m_unpatch;
};

extern spi_patcher spi_patcher_inst;

extern "C" void spi_patcher_inst_request(int8_t id, void (*patch)(spi_host_device_t), void (*unpatch)(spi_host_device_t));

#else

void spi_patcher_inst_request(int8_t id, void (*patch)(spi_host_device_t), void (*unpatch)(spi_host_device_t));

#endif
