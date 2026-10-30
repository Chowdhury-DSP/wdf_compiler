//
// nullAssignment.cpp
//
// Code generation for function 'nullAssignment'
//

// Include files
#include "nullAssignment.h"
#include <cstring>

// Function Definitions
namespace coder {
namespace internal {
void b_nullAssignment(double x_data[], int x_size[2], int idx)
{
  int ncolx;
  int nrows;
  ncolx = x_size[1];
  nrows = x_size[0];
  for (int j{0}; j < ncolx; j++) {
    for (int i{idx}; i < nrows; i++) {
      int b_i;
      b_i = i + x_size[0] * j;
      x_data[b_i - 1] = x_data[b_i];
    }
  }
  nrows = x_size[0] - 1;
  for (int j{0}; j < ncolx; j++) {
    for (int i{0}; i < nrows; i++) {
      x_data[i + nrows * j] = x_data[i + x_size[0] * j];
    }
  }
  x_size[0] = nrows;
}

void nullAssignment(double x_data[], int x_size[2], int idx)
{
  for (int j{idx}; j < 4; j++) {
    int i;
    i = 3 * (j - 1);
    x_data[i] = x_data[3 * j];
    x_data[i + 1] = x_data[3 * j + 1];
    x_data[i + 2] = x_data[3 * j + 2];
  }
  x_size[0] = 3;
  x_size[1] = 3;
}

} // namespace internal
} // namespace coder

// End of code generation (nullAssignment.cpp)
