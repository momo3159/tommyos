#include "window.hpp"
#include "logger.hpp"

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