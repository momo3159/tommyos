#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <numeric>
#include <vector>
#include <deque>
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

unsigned int mouse_layer_id;
Vector2D<int> screen_size;
Vector2D<int> mouse_position;
void MouseObserver(uint8_t buttons, int8_t displacement_x, int8_t displacement_y) {
  static unsigned int mouse_drag_layer_id = 0;
  static uint8_t previous_buttons = 0;

  const auto oldpos = mouse_position;
  auto newpos = mouse_position + Vector2D<int>{displacement_x, displacement_y};
  newpos = ElementMin(newpos, screen_size + Vector2D<int>{-16, -24});
  mouse_position = ElementMax(newpos, {0, 0});

  const auto pos_diff = mouse_position - oldpos;
  layer_manager->Move(mouse_layer_id, mouse_position);

  const bool previous_left_pressed = (previous_buttons & 0x01);
  const bool left_pressed = (buttons & 0x01);
  if (!previous_left_pressed && left_pressed) {
    // 左ボタンを押したとき
    auto layer = layer_manager->FindLayerByPosition(mouse_position, mouse_layer_id);
    if (layer && layer->IsDraggable()) {
      mouse_drag_layer_id = layer->ID();
    }
  } else if (previous_left_pressed && left_pressed) {
    // 左ボタンを押し続けているとき
    if (mouse_drag_layer_id > 0) {
      layer_manager->MoveRelative(mouse_drag_layer_id, pos_diff);
    }
  } else if (previous_left_pressed && !left_pressed) {
    // 左ボタンを離したとき
    mouse_drag_layer_id = 0;
  }

  previous_buttons = buttons;
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
  usb::HIDMouseDriver::default_observer = MouseObserver;


  FrameBuffer screen;
  if (auto err = screen.Initialize(frame_buffer_config)) {
    Log(kError, "failed to initialize frame buffer: %s at %s:%d\n", 
      err.Name(), err.File(), err.Line()
    );
  }

  screen_size.x = frame_buffer_config.horizontal_resolution; 
  screen_size.y = frame_buffer_config.vertical_resolution;

  layer_manager = new LayerManager;
  layer_manager->SetWriter(&screen);
  
  auto bgwindow = std::make_shared<Window>(screen_size.x, screen_size.y, frame_buffer_config.pixel_format);
  DrawDesktop(*bgwindow->Writer());
  
  auto mouse_window = std::make_shared<Window>(
    kMouseCursorWidth, kMouseCursorHeight, frame_buffer_config.pixel_format
  );
  mouse_window->SetTransparentColor(kMouseTransparentColor);
  DrawMouseCursor(mouse_window->Writer(), {0, 0});
  mouse_position = {200, 200};

  auto main_window = std::make_shared<Window>(
    160, 52, frame_buffer_config.pixel_format
  );
  DrawWindow(*main_window->Writer(), "Hello Window");

  auto console_window = std::make_shared<Window>(
    Console::kColumns * 8, Console::kRows * 16, frame_buffer_config.pixel_format
  );
  console->SetWindow(console_window);

  auto bglayer_id = layer_manager->NewLayer()
    .SetWindow(bgwindow)
    .Move({0, 0})
    .ID();
  console->SetLayerID(layer_manager->NewLayer()
    .SetWindow(console_window)
    .Move({0, 0})
    .ID()
  );

  mouse_layer_id = layer_manager->NewLayer()
    .SetWindow(mouse_window)
    .Move(mouse_position)
    .ID();
  auto main_window_layer_id = layer_manager->NewLayer()
    .SetWindow(main_window)
    .SetDraggable(true)
    .Move({200, 100})
    .ID();
  
  layer_manager->UpDown(bglayer_id, 0);
  layer_manager->UpDown(console->LayerID(), 1);
  layer_manager->UpDown(main_window_layer_id, 2);
  layer_manager->UpDown(mouse_layer_id, 3);
  layer_manager->Draw(bglayer_id);

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