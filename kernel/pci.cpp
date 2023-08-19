#include "pci.hpp"
#include "asmfunc.hpp"

uint32_t MakeAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset) {
  auto shl = [](uint32_t x, unsigned int bits) {
    return x << bits;
  };

  return shl(1, 31)
    | shl(bus, 16)
    | shl(device, 11)
    | shl(function, 8)
    | (reg_offset & 0xfc); // NOTE: オフセットは4バイト単位（4の倍数）→下位2ビットが0になるようにマスクする
}


void WriteAddress(uint32_t address) {
  IoOut32(kConfigAddress, address);
}

void WriteData(uint32_t data) {
  IoOut32(kConfigData, data);
}

uint32_t ReadData() {
  IoIn32(kConfigData);
}

uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function) {
  auto address = MakeAddress(bus, device, function, 0x00);
  WriteAddress(address);
  return ReadData() & 0xffffu;
}

uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function) {
  auto address = MakeAddress(bus, device, function, 0x00);
  WriteAddress(address);
  return ReadData() >> 16;
}

uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
  auto address = MakeAddress(bus, device, function, 0x0c);
  WriteAddress(address);
  return (ReadData() >> 16) & 0xffu;
}

uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function) {
  auto address = MakeAddress(bus, device, function, 0x08);
  WriteAddress(address);
  return ReadData();
}

uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function) {
  auto address = MakeAddress(bus, device, function, 0x18);
  WriteAddress(address);
  return ReadData();
}
