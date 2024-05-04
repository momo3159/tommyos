#include <cstdint>
#include "frame_buffer/frame_buffer_config.hpp"

struct PixelColor {
  uint8_t r, g, b;
};

class PixelWriter {
  public:
    PixelWriter(const FrameBufferConfig& config) : config_{config} {} // コンストラクタ
    virtual ~PixelWriter() = default; // デストラクタ
    virtual void Write(int x, int y, const PixelColor& c) = 0; // 0 は純粋仮想関数を表す == 実装を持たない。

  protected:
    uint8_t* PixelAt(int x, int y) {
      return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x);
    }
  
  private:
    const FrameBufferConfig& config_;
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter; // コンストラクタは親のものを使う
    
    virtual void Write(int x, int y, const PixelColor& c) override {
      auto p = PixelAt(x, y);
      p[0] = c.b; 
      p[1] = c.g; 
      p[2] = c.r;
    }
};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter; // コンストラクタは親のものを使う
    
    virtual void Write(int x, int y, const PixelColor& c) override {
      auto p = PixelAt(x, y);
      p[0] = c.r; 
      p[1] = c.g; 
      p[2] = c.b;
    }
};



int WritePixel(const FrameBufferConfig& config, int x, int y, const PixelColor& c) {
  uint8_t* p = &config.frame_buffer[4 * (x + y * config.pixels_per_scan_line)];
  if (config.pixel_format == kPixelBGRResv8BitPerColor) {
    p[0] = c.b; p[1] = c.g; p[2] = c.r;
  } else if(config.pixel_format == kPixelRGBResv8BitPerColor) {
    p[0] = c.r; p[1] = c.g; p[2] = c.b;
  } else {
    return -1;
  }

  return 0;
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  for (int x=0;x<frame_buffer_config.horizontal_resolution;x++) {
    for (int y=0;y<frame_buffer_config.vertical_resolution;y++) {
      WritePixel(frame_buffer_config, x, y, {255, 255, 255});
    }
  }

  for (int x=0;x<200;x++) {
    for (int y=0;y<100;y++) {
      WritePixel(frame_buffer_config, 100 + x, 100 + y, {0, 255, 0});
    }
  }
  while (1) __asm__("hlt");
}

