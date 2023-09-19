#include "window.hpp"
#include "logger.hpp"
#include "font.hpp"

Window::Window(int width, int height, PixelFormat shadow_format) : width_{width}, height_{height} {
  data_.resize(height);
  for (int y=0;y<height;y++) data_[y].resize(width);

  FrameBufferConfig config{};
  config.frame_buffer = nullptr;
  config.horizontal_resolution = width_;
  config.vertical_resolution = height_;
  config.pixel_format = shadow_format;

  if (auto err = shadow_buffer_.Initialize(config)) {
    Log(kError, "failed to initialize shadow buffer: %s at %s:%d\n", 
      err.Name(), err.File(), err.Line()
    );
  }
}

void Window::Write(Vector2D<int> pos, PixelColor c) {
  data_[pos.y][pos.x] = c;
  shadow_buffer_.Writer().Write(pos, c);
}

void Window::DrawTo(FrameBuffer& dst, Vector2D<int> position) {
  if (!transparent_color_) {
    dst.Copy(position, shadow_buffer_);
    return;
  }

  const auto tc = transparent_color_.value();
  auto& writer = dst.Writer();

  // srcの始点（position）を原点としたときのdst.Writerの座標で考える
  for (int y = std::max(0, 0 - position.y);
       y < std::min(Height(), writer.Height() - position.y);
       ++y) {
    for (int x = std::max(0, 0 - position.x);
         x < std::min(Width(), writer.Width() - position.x);
         ++x)  {
      const auto c = At(Vector2D<int>{x, y});
      if (c != tc) {
        writer.Write(position + Vector2D<int>{x, y}, c);
      }
    }
  }
}

int Window::Height() const {
  return height_;
}

int Window::Width() const {
  return width_;
}

const PixelColor& Window::At(Vector2D<int> pos) const {
  return data_[pos.y][pos.x];
}

Window::WindowWriter* Window::Writer() {
  return &writer_;
}

void Window::SetTransparentColor(std::optional<PixelColor> c) {
  transparent_color_ = c;
}

void Window::Move(Vector2D<int> dst_pos, const Rectangle<int>& src) {
  // TODO: data_の更新は必要？不要？
  MovePixelColors(dst_pos, src);
  shadow_buffer_.Move(dst_pos, src);
}

void Window::MovePixelColors(Vector2D<int> dst_pos, const Rectangle<int>& src) {
    if (dst_pos.y < src.pos.y) {
      for (int dy=0;dy<src.size.y;dy++) {
        auto dst_row = data_[dst_pos.y + dy];
        auto dst_start_x = dst_row.begin() + dst_pos.x;
        auto dst_end_x   = dst_row.begin() + dst_pos.x + src.size.x;

        auto src_row = data_[src.pos.y + dy];
        auto src_start_x = src_row.begin() + src.pos.x;
        auto src_end_x   = src_row.begin() + src.pos.x + src.size.x;
        
        dst_row.erase(dst_start_x, dst_end_x);
        dst_row.insert(dst_row.begin() + dst_pos.x, src_start_x, src_end_x);
      }
    } else {
      for (int dy=0;dy<src.size.y;dy++) {   
        auto dst_row = data_[dst_pos.y + dy];
        auto dst_start_x = dst_row.begin() + dst_pos.x;
        auto dst_end_x   = dst_row.begin() + dst_pos.x + src.size.x;

        auto src_row = data_[src.pos.y + dy];
        auto src_start_x = src_row.begin() + src.pos.x;
        auto src_end_x   = src_row.begin() + src.pos.x + src.size.x;
        
        dst_row.erase(dst_start_x, dst_end_x);
        dst_row.insert(dst_row.begin() + dst_pos.x, src_start_x, src_end_x);
      }
    }
  }


namespace {
  const int kCloseButtonWidth = 16;
  const int kCloseButtonHeight = 14;
  const char close_button[kCloseButtonHeight][kCloseButtonWidth + 1] = {
    "...............@",
    ".:::::::::::::$@",
    ".:::::::::::::$@",
    ".:::@@::::@@::$@",
    ".::::@@::@@:::$@",
    ".:::::@@@@::::$@",
    ".::::::@@:::::$@",
    ".:::::@@@@::::$@",
    ".::::@@::@@:::$@",
    ".:::@@::::@@::$@",
    ".:::::::::::::$@",
    ".:::::::::::::$@",
    ".$$$$$$$$$$$$$$@",
    "@@@@@@@@@@@@@@@@",
  };
  
  constexpr PixelColor ToColor(uint32_t c) {
    return {
      static_cast<uint8_t>((c >> 16) & 0xff),
      static_cast<uint8_t>((c >> 8) & 0xff),
      static_cast<uint8_t>(c & 0xff)
    };
  }

}

void DrawWindow(PixelWriter& writer, const char* title) {
  auto fill_rect = [&writer](Vector2D<int> pos, Vector2D<int> size, uint32_t c) {
    FillRectangle(writer, pos, size, ToColor(c));
  };

  const auto win_w = writer.Width();
  const auto win_h = writer.Height();

  // 上・左のふち
  fill_rect({0, 0}, {win_w, 1}, 0xc6c6c6); fill_rect({0, 0}, {1, win_h}, 0xc6c6c6);
  fill_rect({1, 1}, {win_w - 2, 1}, 0xffffff); fill_rect({1, 1}, {1, win_h - 2}, 0xffffff);

  // 下・右のふち
  fill_rect({win_w - 2, 1}, {1, win_h - 2}, 0x848484); fill_rect({1, win_h - 2}, {win_w - 2, 1}, 0x848484);
  fill_rect({win_w - 1, 0}, {1, win_h}, 0x000000); fill_rect({0, win_h - 1}, {win_w, 1}, 0x000000); 

  // メイン部分
  fill_rect({2, 2}, {win_w - 4, win_h - 4}, 0xc6c6c6);

  // タイトルバー
  fill_rect({3, 3}, {win_w - 6, 18}, 0x000084);
  WriteString(writer, {24, 4}, title, ToColor(0xffffff));

  // 閉じるボタン
  for (int y=0;y<kCloseButtonHeight;y++) {
    for (int x=0;x<kCloseButtonWidth;x++) {
      PixelColor c = ToColor(0xffffff);
      if (close_button[y][x] == '@') c = ToColor(0x000000);
      else if (close_button[y][x] == '$') c = ToColor(0x848484);
      else if (close_button[y][x] == ':') c = ToColor(0xc6c6c6);

      writer.Write({win_w - 5 - kCloseButtonWidth + x, 5 + y}, c);
    }
  }
}