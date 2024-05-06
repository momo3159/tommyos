#include "pci.hpp"

uint32_t MakeAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
  auto shl = [](uint32_t x, unsigned int bits) {
    return x << bits;
  };

  return shl(1, 31) 
      | shl(bus, 16) 
      | shl(device, 11) 
      | shl(function, 8) 
      | (offset & 0xfcu);
  
}