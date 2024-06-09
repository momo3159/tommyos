#include "mouse.hpp"

namespace {
  const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
  };
}


void DrawMouseCursor(PixelWriter* pixel_writer, Vector2D<int> position) {
  for (int dx=0;dx<kMouseCursorWidth;dx++) {
    for (int dy=0;dy<kMouseCursorHeight;dy++) {
      switch (mouse_cursor_shape[dy][dx]) {
        case '@':
          pixel_writer->Write(position.x + dx, position.y + dy, BLACK);
          break;
        case '.':
          pixel_writer->Write(position.x + dx, position.y + dy, WHITE);
          break;
        default:
          pixel_writer->Write(position.x + dx, position.y + dy, kMouseTransparentColor);
      } 
    }
  }
}