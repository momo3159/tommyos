#include "pci.hpp"
#include "asmfunc.h"

namespace {
  // プライベート
  using namespace pci;

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

  
  Error AddDevice(uint8_t bus, uint8_t device, uint8_t function, uint8_t header_type) {
    if (num_device == devices.size()) return MAKE_ERROR(Error::kFull);

    devices[num_device] = Device{bus, device, function, header_type};
    ++num_device;
    return MAKE_ERROR(Error::kSuccess);
  }

  Error ScanBus(uint8_t bus);

  Error ScanFunction(uint8_t bus, uint8_t device, uint8_t function) {
    auto header_type = ReadHeaderType(bus, device, function);
    if (auto err = AddDevice(bus, device, function, header_type)) {
      return err;
    }

    auto class_code = ReadClassCode(bus, device, function);

    if (class_code.Match(0x06u, 0x04u)) {
      // PCI-PCI
      auto bus_numbers = ReadBusNumbers(bus, device, function);
      uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
      return ScanBus(secondary_bus);
    }
    return MAKE_ERROR(Error::kSuccess);
  }

  Error ScanDevice(uint8_t bus, uint8_t device) {
    if (auto err = ScanFunction(bus, device, 0)) {
      return err;
    }
    if (IsSingleFunctionDevice(ReadHeaderType(bus, device, 0))) {
      return MAKE_ERROR(Error::kSuccess);
    }

    for (uint8_t function=1;function<8;function++) {
      if (ReadVendorId(bus, device, function) == 0xffffu) {
        continue;
      }
      if (auto err = ScanFunction(bus, device, function)) {
        return err;
      }
    }
    return MAKE_ERROR(Error::kSuccess);
  }

  Error ScanBus(uint8_t bus) {
    for (uint8_t device=0;device<32;device++) {
      if (ReadVendorId(bus, device, 0) == 0xffffu) {
        continue;
      }
      if (auto err = ScanDevice(bus, device)) {
        return err;
      }
    }
    return MAKE_ERROR(Error::kSuccess);
  }
}

namespace pci {
  void WriteAddress(uint32_t address) {
    IoOut32(kConfigAddress, address);
  }

  void WriteData(uint32_t value) {
    IoOut32(kDataAddress, value);
  }

  uint32_t ReadData() {
    return IoIn32(kDataAddress);
  }

  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x00));
    return ReadData() & 0xffffu;
  }

  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x00));
    return (ReadData() >> 16) & 0xffffu;
  }

  uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x0C));
    return (ReadData() >> 16) & 0xffu;
  }
  

  ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x08));
    auto reg = ReadData();

    ClassCode cc;
    cc.base = (reg >> 24) & 0xffu;
    cc.sub = (reg >> 16) & 0xffu;
    cc.interface = (reg >> 8) & 0xffu;
    return cc;
  }

  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x18));
    return ReadData();
  }
  
  uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr) {
    WriteAddress(MakeAddress(dev.bus, dev.device, dev.function, reg_addr));
    return ReadData();
  }

  Either<uint64_t> ReadBar(Device& dev, unsigned int bar_index) {
    if (bar_index >= 6) {
      return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
    }

    const auto addr = CalcBarAddress(bar_index);
    const auto bar = ReadConfReg(dev, addr);

    // 下位４ビットはフラグ。3ビット目が0なら32bitアドレス
    if ((bar & 4u) == 0) {
      return {bar, MAKE_ERROR(Error::kSuccess)};
    }

    if (bar_index >= 5) {
      return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
    }

    const auto bar_upper = ReadConfReg(dev, addr + 4);
    return {
      bar | (static_cast<uint64_t>(bar_upper) << 32),
      MAKE_ERROR(Error::kSuccess)
    };
  }

  bool IsSingleFunctionDevice(uint8_t header_type) {
    return (header_type >> 7) != 1;
  }

  Error ScanAllBus() {
    /**
     * ホストブリッジのファンクションをチェックし、有効であればバスがあるので探索する
    */
    num_device = 0;

    // ホストブリッジ(CPUとPCIデバイス間の通信を橋渡しする)
    // ファンクションn → バスnを担当する。これより、バスは0 ~ 7の8通りしかない。
    auto header_type = ReadHeaderType(0, 0, 0);
    if (IsSingleFunctionDevice(header_type)) {
      return ScanBus(0);
    }

    for (uint8_t function=0;function<8;function++) {
      if (ReadVendorId(0, 0, function) == 0xffffu) continue;
      if (auto err = ScanBus(function)) return err;
    }

    return MAKE_ERROR(Error::kSuccess);
  }
}










