#!/bin/bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [[ -n "$1" ]]; then
    cpp_compiler="$1"
else
    cpp_compiler="clang"
fi

if [[ "$*" = *bench* ]]; then
    echo "Making running bench tests..."
    bench_flags="-DRUN_BENCH=1 -O3"
else
    bench_flags=""
fi

if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
   wdf_compiler="${SCRIPT_DIR}/../compiler/wdf_compiler.exe"
   libcpp_flag=""
else
   wdf_compiler="${SCRIPT_DIR}/../compiler/wdf_compiler"
   libcpp_flag="-lstdc++"
fi

cpp_test () {
   test="$1"
   echo "Running CPP Test: $test"
   cd "${SCRIPT_DIR}/${test}"
   $wdf_compiler "${test}.wdf" "${test}.h"
   $cpp_compiler "${test}.cpp" ${bench_flags} -I../../lib --std=c++20 ${libcpp_flag} -o "${test}.exe"
   "./${test}.exe"
}

if [[ "$*" = *bench* ]]; then
   cpp_test preamp_eq_comb
   cpp_test diode_clipper
   cpp_test simple_triode
   cpp_test baxandall_eq
else
   cpp_test rc_lowpass
   cpp_test rc_bandpass
   cpp_test rl_lowpass
   cpp_test rc_lowpass_var
   cpp_test rl_lowpass_var
   cpp_test rc_bandpass_var
   cpp_test rc_lowpass_2ins
   cpp_test preamp_eq
   cpp_test preamp_eq_comb
   cpp_test diode_clipper
   cpp_test simple_triode
   cpp_test bassman_tone_stack
   cpp_test baxandall_eq
fi


if [[ "$*" = *bad_configs* ]]; then
    echo "Running bad config tests..."
    cd "${SCRIPT_DIR}/bad_configs"
    python3 run_tests.py
fi
