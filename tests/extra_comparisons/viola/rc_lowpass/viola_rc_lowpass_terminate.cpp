//
// viola_rc_lowpass_terminate.cpp
//
// Code generation for function 'viola_rc_lowpass_terminate'
//

// Include files
#include "viola_rc_lowpass_terminate.h"
#include "viola_rc_lowpass.h"
#include "viola_rc_lowpass_data.h"
#include <cstring>

// Function Definitions
void viola_rc_lowpass_terminate()
{
  viola_rc_lowpass_delete();
  viola_rc_lowpass_emx_free();
  isInitialized_viola_rc_lowpass = false;
}

// End of code generation (viola_rc_lowpass_terminate.cpp)
