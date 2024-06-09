#include <cstring>
#include "console.hpp"
#include "../font/font.hpp"
#include "../layer/layer.hpp"

Console::Console(const PixelColor& fg_color, const PixelColor& bg_color) : 
  writer_{nullptr}, 
  fg_color_{fg_color},
  bg_color_{bg_color}, 
  buffer_{}, 
  cursor_row_{0}, 
  cursor_column_{0} {}

void Console::PutString(const char* s) {
  while (*s) {
    if (*s == '\n' || cursor_column_ >= kColumns) {
      Newline();
    } else {
      WriteAscii(*writer_, 8 * cursor_column_, 16 * cursor_row_, *s, fg_color_);
      buffer_[cursor_row_][cursor_column_] = *s;
      cursor_column_++;
    }

    s++;
  }

  if (layer_manager) {
    layer_manager->Draw();
  }
}

void Console::Newline() {
  cursor_column_ = 0;
  if (cursor_row_ + 1 < kRows) ++cursor_row_;
  else {
    //一行スクロール
    for (int y=0;y<16*kRows;y++) {
      for (int x=0;x<8*kColumns;x++) {
        writer_->Write(x, y, bg_color_);
      }
    }
    for (int row=0;row<kRows-1;row++) {
      memcpy(buffer_[row], buffer_[row + 1], kColumns + 1);
      WriteString(*writer_, 0, 16 * row, buffer_[row], fg_color_);
    }
    memset(buffer_[kRows-1], 0, kColumns + 1);
  }
}

void Console::SetWriter(PixelWriter* writer) {
  if (writer == writer_) return;

  writer_ = writer;
  Refresh();
}

void Console::Refresh() {
  for (int row=0;row<kRows;row++) {
    WriteString(*writer_, 0, 16 * row, buffer_[row], fg_color_);
  }
}