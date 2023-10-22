#pragma once 
#include <array>
#include <limits>
#include <sys/types.h>
#include "error.hpp"
#include "memory_map.hpp"

namespace {
  constexpr unsigned long long operator"" _KiB(unsigned long long kib) {
    return kib * 1024;
  }
  constexpr unsigned long long operator"" _MiB(unsigned long long mib) {
    return mib * 1024_KiB;
  }
  constexpr unsigned long long operator"" _GiB(unsigned long long gib) {
    return gib * 1024_MiB;
  }
}

// 物理フレーム1つの大きさ（Byte）
static const auto kBytesPerFrame = 4_KiB;

// ページフレーム番号
class FrameID {
  public:
    explicit FrameID(size_t id) : id_{id} {}
    size_t ID() const { return id_; }
    void* Frame() const { return reinterpret_cast<void*>(id_ * kBytesPerFrame); }
  private:
    size_t id_;
};

// 未定義のページフレーム番号を表す定数
static const FrameID kNullFrame{std::numeric_limits<size_t>::max()};

class BitmapMemoryManager {
  public:
    // メモリ管理クラスで扱える最大の物理メモリ量
    static const auto kMaxPhysicalMemoryBytes{128_GiB};
    static const auto kFrameCount{kMaxPhysicalMemoryBytes / kBytesPerFrame};

    // ビットマップ配列の要素型
    using MapLineType = unsigned long;
    static const size_t kBitsPerMapLine{8 * sizeof(MapLineType)};

    BitmapMemoryManager();

    WithError<FrameID> Allocate(size_t num_frames);
    Error Free(FrameID start_frame, size_t num_frames);
    void MarkAllocated(FrameID start_frame, size_t num_frames);

    /*
      メモリマネージャで扱うメモリ範囲を設定する
      この呼び出し以降, Allocateによるメモリ割り当ては設定された範囲内でのみ行われる
    */
    void SetMemoryRange(FrameID range_begin, FrameID range_end);

  private:
    std::array<MapLineType, kFrameCount / kBitsPerMapLine> alloc_map_;
    FrameID range_begin_;
    FrameID range_end_; // 最終フレームの次のフレーム
    bool GetBit(FrameID frame) const;
    void SetBit(FrameID frame, bool allocated);
};

extern BitmapMemoryManager* memory_manager;
void InitializeMemoryManager(const MemoryMap& memory_map);