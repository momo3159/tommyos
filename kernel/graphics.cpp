#include "graphics.hpp"


void BGRResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
  auto p = PixelAt(pos);
  p[0] = c.b; p[1] = c.g; p[2] = c.r;
}


void RGBResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
  auto p = PixelAt(pos);
  p[0] = c.r; p[1] = c.g; p[2] = c.b;
}

FrameBufferConfig screen_config;
PixelWriter* screen_writer;

namespace {
  char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
}

FrameBufferConfig screen_config;
PixelWriter* screen_writer;

Vector2D<int> ScreenSize() {
  return {
    static_cast<int>(screen_config.horizontal_resolution),
    static_cast<int>(screen_config.vertical_resolution)
  };
}

void InitializeGraphics(const FrameBufferConfig& config) {
  ::screen_config = config;

  switch (frame_buffer_config.pixel_format) {
    case kPixelBGRResv8BitPerColor:
      ::pixel_writer = new(pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    case kPixelRGBResv8BitPerColor:
      ::pixel_writer = new(pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    default:
      exit(1);
  }
}

void FillRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
  for (int dy=0;dy<size.y;dy++) {
    for (int dx=0;dx<size.x;dx++) {
      writer.Write(pos + Vector2D<int>{dx, dy}, c);
    }
  }
}

void DrawRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
  for (int dx=0;dx<size.x;dx++) {
    writer.Write(pos + Vector2D<int>{dx, 0}, c);
    writer.Write(pos + Vector2D<int>{dx, size.y}, c);
  }

  for (int dy=0;dy<size.y;dy++) {
    writer.Write(pos + Vector2D<int>{0, dy}, c);
    writer.Write(pos + Vector2D<int>{size.x, dy}, c);
  }
}

void DrawDesktop(PixelWriter& writer) {
  const auto width = writer.Width();
  const auto height = writer.Height();

  FillRectangle(writer, {0, 0}, {width, height - 50}, kDesktopBGColor);
  FillRectangle(writer, {0, height - 50}, {width, 50}, {1, 8, 17});
  FillRectangle(writer, {0, height - 50}, {width / 5, 50}, {80, 80, 80});
  DrawRectangle(writer, {10, height - 40}, {30, 30}, {160, 160, 160});
}