#include <cstring>
#include "console.hpp"
#include "font.hpp"
#include "layer.hpp"

Console::Console(
  const PixelColor& fg_color_, 
  const PixelColor& bg_color_) 
: writer{nullptr}, window_{}, fg_color{fg_color_}, bg_color{bg_color_},
  buffer{}, cursor_row{0}, cursor_column{0} {}

void Console::PutString(const char* s) {
  while(*s) {
    if (*s == '\n') {
      NewLine(); 
    } else if (cursor_column < kColumns-1) {
      WriteAscii(*writer, Vector2D<int>{8 * cursor_column, 16 * cursor_row}, *s, fg_color);
      buffer[cursor_row][cursor_column] = *s;
      ++cursor_column; 
    }
    ++s;
  }

  if (layer_manager) layer_manager->Draw();
}

void Console::NewLine() {
  cursor_column = 0;
  if (cursor_row < kRows - 1) {
    // NOTE: == kRows の場合、次の行に空きがないので不可
    ++cursor_row;
    return;
  } 
  
  if (window_) {
    Rectangle<int> move_src{{0, 16}, {8 * kColumns, 16 * (kRows - 1)}};
    window_->Move({0, 0}, move_src);
    FillRectangle(*writer, {0, 16 * (kRows - 1)}, {8 * kColumns, 16}, bg_color);
  } else {
    FillRectangle(*writer, {0, 0}, {8 * kColumns, 16 * kRows}, bg_color);

    for (int row=0;row<kRows-1;++row) {
      memcpy(buffer[row], buffer[row+1], kColumns+1);
      WriteString(*writer, Vector2D<int>{0, 16 * row}, buffer[row], fg_color);
    }
    memset(buffer[kRows-1], 0, kColumns+1);
  }
}

void Console::Refresh() {
  for (int row=0;row<kRows;row++) {
    WriteString(*writer, Vector2D<int>{0, 16 * row}, buffer[row], fg_color);
  }
}

void Console::SetWriter(PixelWriter* writer_) {
  if (writer == writer_) return;
  writer = writer_;
  window_.reset();
  Refresh(); // 切り替え先に現在のバッファの文字列を書き込む
}

void Console::SetWindow(const std::shared_ptr<Window>& window) {
  if (window_ == window) return;
  
  window_ = window;
  writer  = window->Writer();
  Refresh();
}