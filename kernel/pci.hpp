#pragma once
#include <cstdint>
#include <array>
#include "error.hpp"


namespace pci {
  // IOポートアドレス
  const uint16_t kConfigAddress = 0x0cf8;
  const uint16_t kConfigData    = 0x0cfc;

  void WriteAddress(uint32_t address);
  void WriteData(uint32_t data);
  uint32_t ReadData();


  struct ClassCode {
    uint8_t base, sub, interface;

    bool Match(uint8_t b) { return b == base; }
    bool Match(uint8_t b, uint8_t s) { return Match(b) && s == sub; }
    bool Match(uint8_t b, uint8_t s, uint8_t i) {
      return Match(b, s) && i == interface;
    }
  };

  struct Device {
    uint8_t bus, device, function, header_type;
    ClassCode class_code;
  };


  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
  uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
  ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);
  WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);
  uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);

  inline uint16_t ReadVendorId(const Device& dev) {
    return ReadVendorId(dev.bus, dev.device, dev.function);
  }
 
  constexpr uint8_t CalcBarAddress(unsigned int bar_index) {
    return 0x10 + 4 * bar_index;
  }

 void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

 
  // NOTE: ヘッダファイルでグローバル変数を定義する場合はinlineをつける
  inline std::array<Device, 32> devices;
  inline int num_device;

  bool IsSingleFunctionDevice(uint8_t header_type);
  Error ScanAllBus();
}

