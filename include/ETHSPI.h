#pragma once

#include <Arduino.h>
//#include <esp_eth.h>

class ETHSPIClass
{
private:
    esp_eth_handle_t eth_handle;

public:
    ETHSPIClass();
    void begin();
    String macAddress();
};

extern ETHSPIClass ETHSPI;
