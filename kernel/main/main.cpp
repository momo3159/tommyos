#include <cstdint>
#include <cstddef>
#include <cstdio>
#include "../frame_buffer/frame_buffer_config.hpp"
#include "../graphics/graphics.hpp"
#include "../font/font.hpp"
#include "../console/console.hpp"

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;
char console_buf[sizeof(Console)];
Console* console;

// new 演算子の挙動
// new の第一引数はコンパイラが勝手に埋めてくれる
// メモリ確保したら実際にインスタンス生成をする
void* operator new(size_t sizse, void* buf) {
  return buf;
}

void operator delete(void* obj) noexcept {}


int printk(const char* format, ...) {
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

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

const PixelColor kDesktopBGColor = BLUE;
const PixelColor kDesktopFGColor = WHITE;

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  switch (frame_buffer_config.pixel_format) {
    case kPixelBGRResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf) BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    case kPixelRGBResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf) RGBResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
  }

  for (int x=0;x<frame_buffer_config.horizontal_resolution;x++) {
    for (int y=0;y<frame_buffer_config.vertical_resolution;y++) {
      pixel_writer->Write(x, y, {255, 255, 255});
    }
  }

  for (int x=0;x<200;x++) {
    for (int y=0;y<100;y++) {
      pixel_writer->Write(100 + x, 100 + y, {0, 255, 0});
    }
  }

  console = new(console_buf) Console(*pixel_writer, kDesktopFGColor, kDesktopBGColor);

  const int kFrameWidth = frame_buffer_config.horizontal_resolution;
  const int kFrameHeight = frame_buffer_config.vertical_resolution;

  FillRectangle(*pixel_writer, {0, 0}, {kFrameWidth, kFrameHeight - 50}, kDesktopBGColor); 
  FillRectangle(*pixel_writer, {0, kFrameHeight - 50}, {kFrameWidth, 50}, BLACK);
  FillRectangle(*pixel_writer, {0, kFrameHeight - 50}, {kFrameWidth / 5, 50}, GLAY);
  DrawRectangle(*pixel_writer, {10, kFrameHeight - 40}, {30, 30}, {160, 160, 160});
  for (int dy=0;dy<kMouseCursorHeight;dy++) {
    for (int dx=0;dx<kMouseCursorWidth;dx++) {
      switch (mouse_cursor_shape[dy][dx]) {
        case '@':
          pixel_writer->Write(200 + dx, 100 + dy, BLACK);
          break;
        case '.':
          pixel_writer->Write(200 + dx, 100 + dy, WHITE);
          break;
      }
    }
  }

  printk("Welcome to tommyOS!\n");
  while (1) __asm__("hlt");
}

