#pragma once 
#include <memory>
#include "graphics.hpp"
#include "window.hpp"

class Console {
  public:
    static const int kRows = 25, kColumns = 80;
    
    Console(const PixelColor& fg_color_, const PixelColor& bg_color_);
    void SetWriter(PixelWriter* writer);
    void PutString(const char* s);
    void SetWindow(const std::shared_ptr<Window>& window);
    void SetLayerID(unsigned int id);
    unsigned int LayerID() const;

  private:
    void NewLine();
    void Refresh();

    PixelWriter* writer;
    std::shared_ptr<Window> window_;
    const PixelColor fg_color, bg_color;
    char buffer[kRows][kColumns + 1];
    int cursor_row, cursor_column;
    unsigned int layer_id_;
};

extern Console* console;
void InitializeConsole();