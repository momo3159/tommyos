#pragma once 
#include "../graphics/graphics.hpp"

class Console {
  public:
    static const int kRows = 25, kColumns = 80;
    Console(const PixelColor& fg_color, const PixelColor& bg_color);
    void SetWriter(PixelWriter* writer);
    void PutString(const char* s);
    void Refresh();

  private:
    PixelWriter* writer_;
    const PixelColor fg_color_, bg_color_;
    char buffer_[kRows][kColumns + 1];
    int cursor_row_, cursor_column_;

    void Newline();
};