/**
 * 画像描画関連のプログラムを集めたファイル
 * 複数の.cppファイルで使うプログラム部品はヘッダファイルにおいておく。
*/
#pragma once
#include "../frame_buffer/frame_buffer_config.hpp"


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
    virtual void Write(int x, int y, const PixelColor& c) override;
};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter; // コンストラクタは親のものを使う
    virtual void Write(int x, int y, const PixelColor& c) override;
};