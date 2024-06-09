/**
 * 画像描画関連のプログラムを集めたファイル
 * 複数の.cppファイルで使うプログラム部品はヘッダファイルにおいておく。
*/
#pragma once
#include "../frame_buffer/frame_buffer_config.hpp"


struct PixelColor {
  uint8_t r, g, b;
};

inline bool operator==(const PixelColor& lhs, const PixelColor& rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

inline bool operator!=(const PixelColor& lhs, const PixelColor& rhs) {
  return !(lhs == rhs);
}

static const PixelColor BLACK = PixelColor{0, 0, 0};
static const PixelColor WHITE = PixelColor{255, 255, 255};
static const PixelColor BLUE  = PixelColor{45, 118, 237};
static const PixelColor GLAY  = PixelColor{80, 80, 80};


const PixelColor kDesktopBGColor = BLUE;
const PixelColor kDesktopFGColor = WHITE;

template <typename T>
struct Vector2D {
  T x, y;

  template <typename U>
  Vector2D<T>& operator +=(const Vector2D<U>& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
};

class PixelWriter {
  public:
    virtual ~PixelWriter() = default; // デストラクタ
    virtual void Write(int x, int y, const PixelColor& c) = 0; // 0 は純粋仮想関数を表す == 実装を持たない。
    virtual int Width() const = 0;
    virtual int Height() const = 0;
};


class FrameBufferWriter : public PixelWriter {
  public:
    FrameBufferWriter(const FrameBufferConfig& config) : config_{config} {}
    virtual ~FrameBufferWriter() = default;
    virtual int Width() const override { return config_.horizontal_resolution; }
    virtual int Height() const override { return config_.vertical_resolution; }
    
  protected:
    uint8_t* PixelAt(int x, int y) {
      return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x);
    }

  private:
    const FrameBufferConfig& config_;
};

class BGRResv8BitPerColorPixelWriter : public FrameBufferWriter {
  public:
    using FrameBufferWriter::FrameBufferWriter; // コンストラクタは親のものを使う
    virtual void Write(int x, int y, const PixelColor& c) override;
};

class RGBResv8BitPerColorPixelWriter : public FrameBufferWriter {
  public:
    using FrameBufferWriter::FrameBufferWriter; // コンストラクタは親のものを使う
    virtual void Write(int x, int y, const PixelColor& c) override;
};

void FillRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);
void DrawRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);
void DrawDesktop(PixelWriter& writer);