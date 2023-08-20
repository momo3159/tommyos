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

    Error(Code code_) : code{code_} {}

    operator bool() const {
      return this->code != kSuccess;
    }

    const char* Name() const {
      return code_names[static_cast<int>(this->code)];
    }

  private:
    static constexpr std::array<const char*, 4> code_names = {
      "kSuccess",
      "kFull",
      "kEmpty",
      "kIndexOutOfRange",
    };

    Code code;
};

template <class T>
struct WithError {
  T value;
  Error error;
};