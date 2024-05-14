#pragma once 
#include <cstdint>
#include <array>
#include "../error.hpp"

namespace pci {
  const uint16_t kConfigAddress = 0x0cf8;
  const uint16_t kDataAddress   = 0x0cfc;

  struct ClassCode {
    uint8_t base, sub, interface;

    bool Match(uint8_t b) {
      return b == base;
    }

    bool Match(uint8_t b, uint8_t s) {
      return Match(b) && s == sub;
    }

    bool Match(uint8_t b, uint8_t s, uint8_t i) {
      return Match(b, s) && i == interface;
    }
  };

  struct Device {
    uint8_t bus, device, function, header_type;
    ClassCode class_code;
  };

  inline int num_device;
  inline std::array<Device, 32> devices;

  void WriteAddress(uint32_t address);
  void WriteData(uint32_t value);
  uint32_t ReadData();

  Error ScanAllBus();

  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
  inline uint16_t ReadVendorId(const Device& dev) {
    return ReadVendorId(dev.bus, dev.device, dev.function);
  }
  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
  uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
  ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);
  Either<uint64_t> ReadBar(Device& dev, unsigned int bar_index);

  bool IsSingleFunctionDevice(uint8_t header_type);

  constexpr uint8_t CalcBarAddress(unsigned int bar_index) {
    return 0x10 + bar_index * 4;
  }
}