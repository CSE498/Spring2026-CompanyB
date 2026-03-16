/**
 * @file IExample.hpp
 * @author Scott Haakenson
 * @brief Example interface for demonstration purposes of example class.
 **/

#pragma once
#include <string_view>

namespace cse498 {

class IExample {
public:
  virtual ~IExample() = default;

  /// @brief Example method
  /// @param string Example parameter for demonstration
  virtual void DoSomething(std::string_view string) = 0;
};

}  // namespace cse498
