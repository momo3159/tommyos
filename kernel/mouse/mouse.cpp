#include "mouse.hpp"

namespace {
  const int kMouseCursorHeight = 24;
  const int kMouseCursorWidth = 15;
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

  void DrawMouseCursor(PixelWriter* writer, Vector2D<int> position) {
    for (int dx=0;dx<kMouseCursorWidth;dx++) {
      for (int dy=0;dy<kMouseCursorHeight;dy++) {
        switch (mouse_cursor_shape[dy][dx]) {
          case '@':
            writer->Write(position.x + dx, position.y + dy, BLACK);
            break;
          case '.':
            writer->Write(position.x + dx, position.y + dy, WHITE);
            break;
        } 
      }
    }
  }

  void EraseMouseCursor(PixelWriter* writer, Vector2D<int> position, PixelColor erase_color) {
    for (int dx=0;dx<kMouseCursorWidth;dx++) {
      for (int dy=0;dy<kMouseCursorHeight;dy++) {
        writer->Write(position.x + dx, position.y + dy, erase_color);
      }
    }
  }
}

MouseCursor::MouseCursor(
  PixelWriter* writer, PixelColor erase_color, Vector2D<int> initial_position
) : pixel_writer_{writer}, erase_color_{erase_color}, position_{initial_position} {
  DrawMouseCursor(pixel_writer_, position_);
}

void MouseCursor::MoveRelative(Vector2D<int> displacement) {
  EraseMouseCursor(pixel_writer_, position_, erase_color_);
  position_ += displacement;
  DrawMouseCursor(pixel_writer_, position_);
}
