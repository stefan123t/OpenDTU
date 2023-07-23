#include "ETHSPI.h"

#include <esp_eth.h>
#include <esp_system.h>
#include <FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <Arduino.h>
#include "MessageOutput.h"

extern void tcpipInit();
extern void add_esp_interface_netif(esp_interface_t interface, esp_netif_t* esp_netif); /* from WiFiGeneric */

ETHSPIClass::ETHSPIClass() :
    eth_handle(nullptr)
{

}

void ETHSPIClass::begin(int8_t pin_sclk, int8_t pin_mosi, int8_t pin_miso, int8_t pin_cs, int8_t pin_int, int8_t pin_rst)
{
    delay(1000);

    uint8_t base_mac[6];
    esp_base_mac_addr_get(base_mac);
    base_mac[5] |= 0x03;
    //MessageOutput.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);

    //gpio_reset_pin(static_cast<gpio_num_t>(12));
    //gpio_set_direction(static_cast<gpio_num_t>(12), GPIO_MODE_OUTPUT);
    //gpio_set_level(static_cast<gpio_num_t>(12), 0);

    gpio_set_direction(static_cast<gpio_num_t>(pin_rst), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(pin_rst), 0);

    //gpio_reset_pin(static_cast<gpio_num_t>(39));
    //gpio_set_drive_capability(static_cast<gpio_num_t>(39), GPIO_DRIVE_CAP_3);
    //gpio_reset_pin(static_cast<gpio_num_t>(40));
    //gpio_set_drive_capability(static_cast<gpio_num_t>(40), GPIO_DRIVE_CAP_3);
    //gpio_reset_pin(static_cast<gpio_num_t>(42));
    //gpio_set_drive_capability(static_cast<gpio_num_t>(42), GPIO_DRIVE_CAP_3);
    //gpio_reset_pin(static_cast<gpio_num_t>(41));
    
    //MessageOutput.println("################## 2 #################");

    //ESP_ERROR_CHECK(gpio_install_isr_service(0)); // TODO: Kompatibel? -> offensichtlich nicht ahhhhhhhhhhhhhhhhhhh
    attachInterrupt(digitalPinToInterrupt(11), nullptr, DEFAULT);
    detachInterrupt(digitalPinToInterrupt(11));
    gpio_reset_pin(static_cast<gpio_num_t>(11));

    //MessageOutput.println("################## 3 #################");

    tcpipInit();

    //MessageOutput.println("################## 4 #################");

    //ESP_ERROR_CHECK(tcpip_adapter_set_default_eth_handlers()); // ???????????????????????????????????????????????????????????

    //MessageOutput.println("################## 5 #################");

    spi_bus_config_t buscfg = {
        .mosi_io_num = pin_mosi,
        .miso_io_num = pin_miso,
        .sclk_io_num = pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = 0, // uses default value internally
        .flags = 0,
        .intr_flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO)); // TODO: DMA_CH SPI_DMA_CH_AUTO

    spi_device_handle_t spi;

    spi_device_interface_config_t devcfg = {
        .command_bits = 16, // actually address phase
        .address_bits = 8, // actually command phase
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0, // UNBEDINGT 0 LASSEN
        .cs_ena_posttrans = 0, // UNBEDINGT 0 LASSEN
        .clock_speed_hz = 5000000, // TODO
        .input_delay_ns = 0,
        .spics_io_num = pin_cs,
        .flags = 0,
        .queue_size = 20, // TODO
        .pre_cb = NULL,
        .post_cb = NULL
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &devcfg, &spi));

    delayMicroseconds(500);
    gpio_set_level(static_cast<gpio_num_t>(pin_rst), 1);
    delayMicroseconds(1000);

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi);
    w5500_config.int_gpio_num = pin_int;

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    mac_config.rx_task_stack_size = 8192; // ?
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1; // ???
    phy_config.reset_gpio_num = -1;
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    // ######

    MessageOutput.println("######### 1 #########");
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    if (esp_eth_driver_install(&eth_config, &eth_handle) != ESP_OK) {
        ESP_ERROR_CHECK(mac->del(mac)); // ?
        ESP_ERROR_CHECK(phy->del(phy)); // ?
        MessageOutput.println("######### 2 #########");
        return;
    }
    MessageOutput.println("######### 3 #########");

    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, base_mac));

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_config);

    //MessageOutput.println("################## 11 #################");

    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    //MessageOutput.println("################## 12 #################");

    /* attach to WiFiGeneric to receive events */
    add_esp_interface_netif(ESP_IF_ETH, eth_netif);

    //MessageOutput.println("################## 13 #################");

    esp_err_t err = esp_eth_start(eth_handle);

    //gpio_set_level(static_cast<gpio_num_t>(12), 1);

    //MessageOutput.println("################## 14 #################");

    ESP_ERROR_CHECK(err);

    //MessageOutput.println("################## 15 #################");

    //delay(100);
}

String ETHSPIClass::macAddress()
{
    uint8_t mac_addr[6] = {0, 0, 0, 0, 0, 0};
    esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
    char mac_addr_str[18];
    sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    return String(mac_addr_str);
}

ETHSPIClass ETHSPI;
