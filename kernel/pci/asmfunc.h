#pragma once 
#include <stdint.h>

extern "C" {
  uint32_t IoIn32(uint16_t addr);
  void IoOut32(uint16_t addr, uint32_t data);
}