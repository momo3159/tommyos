#include "frame_buffer.hpp"

Error FrameBuffer::Initialize(const FrameBufferConfig& config) {
  config_ = config;

  const auto bits_per_pixel = BitsPerPixel(config_.pixel_format);
  if (bits_per_pixel <= 0) {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  if (config_.frame_buffer) {
    // frame_bufferにすでに書き込み先がある場合
    buffer_.resize(0);
  } else {
    const auto byte_per_pixel = (bits_per_pixel + 7) / 8;

    buffer_.resize(
      config_.horizontal_resolution * config_.vertical_resolution * byte_per_pixel
    );
    config_.frame_buffer = buffer_.data();
    config_.pixels_per_scan_line = config_.horizontal_resolution;
  }

  switch (config_.pixel_format) {
    case kPixelBGRResv8BitPerColor:
      writer_ = std::make_unique<BGRResv8BitPerColorPixelWriter>(config_);
      break;
    case kPixelRGBResv8BitPerColor:
      writer_ = std::make_unique<RGBResv8BitPerColorPixelWriter>(config_);
      break; 
    default:
      return MAKE_ERROR(Error::kUnknownPixelFormat); 
  }

  return MAKE_ERROR(Error::kSuccess);
}

Error FrameBuffer::Copy(Vector2D<int> pos, const FrameBuffer& src) {
  if (config_.pixel_format != src.config_.pixel_format) {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const auto bits_per_pixel = BitsPerPixel(config_.pixel_format);
  if (bits_per_pixel <= 0) {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const auto dst_width = config_.horizontal_resolution;
  const auto dst_height = config_.vertical_resolution;
  const auto src_width = src.config_.horizontal_resolution;
  const auto src_height = src.config_.vertical_resolution;

  const auto copy_start_dst_x = std::max(0, pos.x);
  const auto copy_start_dst_y = std::max(0, pos.y);
  const auto copy_end_dst_x = std::min(dst_width, pos.x + src_width);
  const auto copy_end_dst_y = std::min(dst_height, pos.y + src_height); 

  const auto bytes_per_pixel = (bits_per_pixel + 7) / 8;
  const auto bytes_per_copy_line = (copy_end_dst_x - copy_start_dst_x) * bytes_per_pixel;

  auto dst_buf = config_.frame_buffer + 
    (config_.pixels_per_scan_line * copy_start_dst_y + copy_start_dst_x) * bytes_per_pixel;
  auto src_buf = src.config_.frame_buffer + 
    (src.config_.pixels_per_scan_line * std::abs(copy_start_dst_y - pos.y) + std::abs(copy_start_dst_x - pos.x)) * bytes_per_pixel;
    
  for (int dy=0;dy<copy_end_dst_y - copy_start_dst_x;dy++) {
    memcpy(dst_buf, src_buf, bytes_per_copy_line);
    dst_buf += bytes_per_pixel * config_.pixels_per_scan_line;
    src_buf += bytes_per_pixel * src.config_.pixels_per_scan_line;
  }

  return MAKE_ERROR(Error::kSuccess);
}

int BitsPerPixel(PixelFormat format) {
  // 対象外のフォーマットの場合-1を返す
  switch (format) {
    case kPixelBGRResv8BitPerColor: return 32;
    case kPixelRGBResv8BitPerColor: return 32;
    default: return -1;
  }
}