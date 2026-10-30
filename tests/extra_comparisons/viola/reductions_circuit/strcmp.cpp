//
// strcmp.cpp
//
// Code generation for function 'strcmp'
//

// Include files
#include "strcmp.h"
#include <cstring>

// Function Definitions
namespace coder {
namespace internal {
bool b_strcmp(const char a[2])
{
  static const char b[2]{'O', 'N'};
  int ret;
  ret = std::memcmp(&a[0], &b[0], 2U);
  return ret == 0;
}

} // namespace internal
} // namespace coder

// End of code generation (strcmp.cpp)
