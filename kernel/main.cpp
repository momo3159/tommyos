#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <numeric>
#include <vector>
#include <deque>
#include <limits>
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "logger.hpp"
#include "mouse.hpp"
#include "interrupt.hpp"
#include "asmfunc.hpp"
#include "queue.hpp"
#include "memory_map.hpp"
#include "segment.hpp"
#include "paging.hpp"
#include "memory_manager.hpp"
#include "window.hpp"
#include "layer.hpp"
#include "timer.hpp"
#include "message.hpp"
#include "usb/memory.hpp"
#include "usb/device.hpp"
#include "usb/classdriver/mouse.hpp"
#include "usb/xhci/xhci.hpp"
#include "usb/xhci/trb.hpp"

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



void InitializeMainWindow() {
  auto main_window = std::make_shared<Window>(
    160, 52, screen_config.pixel_format
  );
  DrawWindow(*main_window->Writer(), "Hello Window");

  auto main_window_layer_id = layer_manager->NewLayer()
    .SetWindow(main_window)
    .SetDraggable(true)
    .Move({200, 100})
    .ID();

  layer_manager->UpDown(main_window_layer_id, std::numeric_limits<int>::max());
}

usb::xhci::Controller* xhc;

std::deque<Message>* main_queue;

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(
  const FrameBufferConfig& frame_buffer_config_ref,
  const MemoryMap& memory_map_ref
) {
  MemoryMap memory_map{memory_map_ref};

  InitializeGraphics(frame_buffer_config_ref);
  InitializeConsole();

  SetLogLevel(kWarn);

  InitializeSegmentation();
  InitializePaging();
  ::main_queue = new std::deque<Message>(32);
  InitializeInterrupt(main_queue);
  InitializePCI();
  usb::xhci::Initialize();

  InitializeLayer();
  InitializeMainWindow();
  InitializeMouse();
  layer_manager->Draw({{0, 0}, ScreenSize()});

  // レイヤの準備が完了する前にもコンソールにログを表示したい
  // そのためまずはフレームバッファに直接書き込み、
  // LayerManagerの準備が終わったタイミングで切り替える
  DrawDesktop(*pixel_writer);

  InitializeLAPICTimer();



  const uint16_t cs = GetCS();

  ::xhc = &xhc;
  __asm__("sti");

  
  std::array<Message, 32> main_queue_data;
  ArrayQueue<Message> main_queue{main_queue_data};
  ::main_queue = &main_queue; // https://rainbow-engine.com/cpp-scope-operator/



  

  char str[128];
  unsigned int count = 0;

  while (true) {
    count++;
    sprintf(str, "%010u", count);
    FillRectangle(*main_window->Writer(), {24, 28}, {8*10, 16}, {0xc6, 0xc6, 0xc6});
    WriteString(*main_window->Writer(), {24, 28}, str, {0, 0, 0});
    layer_manager->Draw(main_window_layer_id);

    __asm__("cli");
    if (main_queue.Count() == 0) {
      __asm__("sti");
      continue;
    }

    Message msg = main_queue.Front();
    main_queue.Pop();
    __asm__("sti");

    switch (msg.type) {
    case Message::kInterruptXHCI:
      while (xhc.PrimaryEventRing()->HasFront()) {
        if (auto err = ProcessEvent(xhc)) {
          Log(kError, "Error while ProcessEvent: %s at %s:%d\n", 
          err.Name(), err.File(), err.Line());
        }
      }
      break;
    
    default:
      Log(kError, "Unknown message type: %d\n", msg.type);
      break;
    }
  }

  while(1) __asm__("hlt");
}


extern "C" void __cxa_pure_virtual() {
  while (1) __asm__("hlt");
}