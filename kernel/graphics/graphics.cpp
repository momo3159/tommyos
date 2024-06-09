#include "graphics.hpp"

void BGRResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor& c) {
  auto p = PixelAt(x, y);
  p[0] = c.b; 
  p[1] = c.g; 
  p[2] = c.r;
}

void RGBResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor& c) {
  auto p = PixelAt(x, y);
  p[0] = c.r; 
  p[1] = c.g; 
  p[2] = c.b;
}

void DrawRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
  for (int dx=0;dx<size.x;dx++) {
    writer.Write(pos.x + dx, pos.y, c);
    writer.Write(pos.x + dx, pos.y + size.y, c);
  }

  for (int dy=0;dy<size.y;dy++) {
    writer.Write(pos.x, pos.y + dy, c);
    writer.Write(pos.x + size.x, pos.y + dy, c);
  }
}

void FillRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
  for (int dy=0;dy<size.y;dy++) {
    for (int dx=0;dx<size.x;dx++) {
      writer.Write(pos.x + dx, pos.y + dy, c);
    }
  }
}

void DrawDesktop(PixelWriter& writer) {
  FillRectangle(writer, {0, 0}, {writer.Width(), writer.Height() - 50}, kDesktopBGColor); 
  FillRectangle(writer, {0, writer.Height() - 50}, {writer.Width(), 50}, BLACK);
  FillRectangle(writer, {0, writer.Height() - 50}, {writer.Width() / 5, 50}, GLAY);
  DrawRectangle(writer, {10, writer.Height() - 40}, {30, 30}, {160, 160, 160});
}