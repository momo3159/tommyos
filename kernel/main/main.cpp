#include <cstdint>
#include <cstddef>
#include <cstdio>
#include "../frame_buffer/frame_buffer_config.hpp"
#include "../graphics/graphics.hpp"
#include "../font/font.hpp"
#include "../console/console.hpp"


// new 演算子の挙動
// new の第一引数はコンパイラが勝手に埋めてくれる
// メモリ確保したら実際にインスタンス生成をする
void* operator new(size_t sizse, void* buf) {
  return buf;
}

void operator delete(void* obj) noexcept {}



char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;
char console_buf[sizeof(Console)];
Console* console;

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

  //   Console console{*pixel_writer, {0, 0, 0}, {255, 255, 255}};

  char buf[128];
  console = new(console_buf) Console(*pixel_writer, {0, 0, 0}, {255, 255, 255});
  console->PutString("ucljqnmmydmvdznccydirzoiyuyyyvnuxotqsuhxqfeyfrisooolvggspxsemzktlnqzpxbjmzdlzpctqwavuimdyztmtwtielmd\n");
  for (int i=0;i<27;i++) {
    sprintf(buf, "line %d\n", i);
    console->PutString(buf);        
  }

  while (1) __asm__("hlt");
}

