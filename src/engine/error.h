#pragma once

#include <functional>

namespace el::engine {

enum class ErrorSeverity {
  kVerbose,
  kInfo,
  kWarning,
  kError,
};

enum class ErrorType {
  kGeneral,
  kValidation,
  kPerformance,
};

struct Error {
  ErrorSeverity severity;
  ErrorType type;
  std::string_view message;
  void* user_data = nullptr;
};

using ErrorCallback = std::function<void(const Error& data)>;
struct ErrorData {
  ErrorCallback cb;
  void* user_data = nullptr;
};

}  // namespace el::engine
