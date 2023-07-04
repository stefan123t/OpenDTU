#include "spi_patcher.h"

spi_patcher spi_patcher_inst(SPI3_HOST);

spi_patcher::spi_patcher(spi_host_device_t host_device) :
    m_host_device(host_device),
    m_id(-1)
{

}

void spi_patcher::request(int8_t id, std::function<void(spi_host_device_t)> patch, std::function<void(spi_host_device_t)> unpatch)
{
    /*if (m_id & (1u << id)) {
        m_id &= ~(1u << id);
        if (id == 0) {
            patch(SPI3_HOST);
        } else if (id == 1) {
            patch(SPI2_HOST);
        }
    }*/
    if (m_id != id) {
        if (m_unpatch.has_value()) {
            m_unpatch.value()(m_host_device);
        }
        patch(m_host_device);
        m_id = id;
        m_unpatch = unpatch;
    }
}

void spi_patcher_inst_request(int8_t id, void (*patch)(spi_host_device_t), void (*unpatch)(spi_host_device_t))
{
    spi_patcher_inst.request(id, patch, unpatch);
}
