#pragma once
#include <cstdint>

// IOポートアドレス
const uint16_t kConfigAddress = 0x0cf8;
const uint16_t kConfigData    = 0x0cfc;

uint32_t MakeAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset); 