#!/bin/bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [[ -n "$1" ]]; then
    cpp_compiler="$1"
else
    cpp_compiler="clang"
fi

wdf_compiler="${SCRIPT_DIR}/../compiler/wdf_compiler"

cpp_test () {
   test="$1"
   echo "Running CPP Test: $test"
   cd "${SCRIPT_DIR}/${test}"
   $wdf_compiler "${test}.wdf" "${test}.h"
   $cpp_compiler "${test}.cpp" --std=c++20 -lstdc++ -o "${test}.exe"
   "./${test}.exe"
}

cpp_test rc_lowpass
cpp_test rc_bandpass
cpp_test rl_lowpass
cpp_test rc_lowpass_var
cpp_test rl_lowpass_var
cpp_test preamp_eq

if [[ "$*" = *bad_configs* ]]; then
    echo "Running bad config tests..."
    cd "${SCRIPT_DIR}/bad_configs"
    python3 run_tests.py
fi
