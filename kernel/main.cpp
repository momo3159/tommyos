#include <cstdint>
#include <cstddef>
#include "frame_buffer/frame_buffer_config.hpp"

const uint8_t kFontA[16] = {
  0b00000000, //________
  0b00011000, //___**___
  0b00011000, //___**___
  0b00011000, //___**___
  0b00011000, //___**___
  0b00100100, //__*__*___
  0b00100100, //__*__*__
  0b00100100, //__*__*__
  0b00100100, //__*__*__
  0b01111110, //_******_
  0b01000010, //_*____*_
  0b01000010, //_*____*_
  0b01000010, //_*____*_
  0b11100111, //***__***_
  0b00000000, //________
  0b00000000, //________
};

// new 演算子の挙動
// new の第一引数はコンパイラが勝手に埋めてくれる
// メモリ確保したら実際にインスタンス生成をする
void* operator new(size_t sizse, void* buf) {
  return buf;
}

void operator delete(void* obj) noexcept {}

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

void WriteAscii(PixelWriter& writer, int x, int y, char c, const PixelColor& color) {
  if (c != 'A') return;

  for (int dx=0;dx<8;dx++) {
    for (int dy=0;dy<16;dy++) {
      if ((kFontA[dy] << dx) & 0x80u) {
        writer.Write(x + dx, y + dy, color);
      }
    }
  }
}

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  switch (frame_buffer_config.pixel_format) {
    case kPixelBGRResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf) BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    case kPixelRGBResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf) RGBResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
  }

  for (int x=0;x<frame_buffer_config.horizontal_resolution;x++) {
    for (int y=0;y<frame_buffer_config.vertical_resolution;y++) {
      pixel_writer->Write(x, y, {255, 255, 255});
    }
  }

  for (int x=0;x<200;x++) {
    for (int y=0;y<100;y++) {
      pixel_writer->Write(100 + x, 100 + y, {0, 255, 0});
    }
  }

  WriteAscii(*pixel_writer, 50, 50, 'A', {0, 0, 0});
  WriteAscii(*pixel_writer, 58, 50, 'A', {0, 250, 0});
  while (1) __asm__("hlt");
}

