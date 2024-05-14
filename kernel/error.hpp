#pragma once
#include <array>

class Error {
  public:
    enum Code {
      kSuccess,
      kFull,
      kEmpty,
      kIndexOutOfRange,
      kLastOfCode,
    };
    
    Error(Code code, const char* file, int line) : code_{code}, line_{line}, file_{file} {}

    operator bool() const {
      return this->code_ != kSuccess;
    }

    Code Cause() const {
      return this->code_;
    }

    const char* Name() const {
      return code_names_[static_cast<int>(this->code_)];
    }

    const char* File() const {
      return this->file_;
    }

    int Line() const {
      return this->line_;
    }

  private:
    static constexpr std::array<const char*, 4> code_names_ = {
      "kSuccess",
      "kFull",
      "kEmpty",
      "kIndexOutOfRange",
    };

    Code code_;
    int line_;
    const char* file_;
};

template <class T>
struct Either {
  T value;
  Error error;
};

// C++ では new を使わなくてもインスタンスを作れる。ただし、ヒープ領域に確保されない。
#define MAKE_ERROR(code) Error((code), __FILE__, __LINE__)