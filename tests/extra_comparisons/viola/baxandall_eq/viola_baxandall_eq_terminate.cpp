//
// viola_baxandall_eq_terminate.cpp
//
// Code generation for function 'viola_baxandall_eq_terminate'
//

// Include files
#include "viola_baxandall_eq_terminate.h"
#include "viola_baxandall_eq.h"
#include "viola_baxandall_eq_data.h"
#include <cstring>

// Function Definitions
void viola_baxandall_eq_terminate()
{
  viola_baxandall_eq_delete();
  viola_baxandall_eq_emx_free();
  isInitialized_viola_baxandall_eq = false;
}

// End of code generation (viola_baxandall_eq_terminate.cpp)
