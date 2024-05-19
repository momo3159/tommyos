#pragma once 
#include <stdint.h>

extern "C" {
  uint32_t IoIn32(uint16_t addr);
  void IoOut32(uint16_t addr, uint32_t data);
  uint16_t GetCS(void);
  void LoadIDT(uint16_t limit, uint64_t offset);
  void LoadGDT(uint16_t limit, uint64_t offset);
  void SetDSAll(uint16_t value);
  void SetCSSS(uint16_t cs, uint16_t ss);
}