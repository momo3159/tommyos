#pragma once
#include <cstdint>
#include <queue>
#include <vector>
#include <limits>
#include "message.hpp"

void InitializeLAPICTimer();
void StartLAPICTimer();
uint32_t LAPICTimerElapsed();
void StopLAPICTimer();
void LAPICTimerOnInterrupt();

class Timer {
  public:
    Timer(unsigned long timeout, int value);
    unsigned long Timeout() const { return timeout_; }
    int Value() const { return value_; }

  private:
    unsigned long timeout_;
    int value_;
};

inline bool operator <(const Timer& lhs, const Timer& rhs) {
  // タイムアウトが近いタイマーを優先的に扱うようにする
  return lhs.Timeout() > rhs.Timeout();
}

class TimerManager {
  public: 
    TimerManager();
    void AddTimer(const Timer& timer);
    bool Tick();
    unsigned long CurrentTick() const { return tick_; }

  private:
    volatile unsigned long long tick_{0};
    std::priority_queue<Timer> timers_{};
};

extern TimerManager* timer_manager;
extern unsigned long lapic_timer_freq;
const int kTimerFreq = 100; // 1秒間に100回tickを呼び出す

const int kTaskTimerPeriod = static_cast<int>(kTimerFreq * 0.02);
const int kTaskTimerValue = std::numeric_limits<int>::min();