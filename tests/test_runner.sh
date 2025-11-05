#!/bin/bash

set -e
shopt -s expand_aliases
source ~/.bashrc

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [[ -n "$1" ]]; then
    cpp_compiler="$1"
else
    cpp_compiler="clang"
fi

if [[ "$*" = *bench* ]]; then
    echo "Running bench tests..."
    if [ ! -d "${SCRIPT_DIR}/xsimd" ]; then
        git clone https://github.com/xtensor-stack/xsimd "${SCRIPT_DIR}/xsimd"
    fi
    bench_flags="-DRUN_BENCH=1 -O3 -I${SCRIPT_DIR}/xsimd/include"
    echo "$bench_flags"
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

jai_test () {
   test="$1"
   echo "Running JAI Test: $test"
   cd "${SCRIPT_DIR}/${test}"
   $wdf_compiler -lang jai "${test}.wdf" "${test}.jai"
   jai "${test}_test.jai"
   "./${test}_test.exe"
}

test () {
   test="$1"
   # The C++ test needs to run first,
   # since it generates the reference data.
   if [[ "$*" = *cpp* ]]; then cpp_test $test; fi
   if [[ "$*" = *jai* ]]; then jai_test $test; fi
}

if [[ "$*" = *bench* ]]; then
   cpp_test preamp_eq_comb
   cpp_test diode_clipper
   cpp_test simple_triode
   cpp_test baxandall_eq
else
   test rc_lowpass cpp jai
   test rc_bandpass cpp jai
   test rl_lowpass cpp jai
   test rc_lowpass_var cpp jai
   test rl_lowpass_var cpp jai
   test rc_bandpass_var cpp jai
   test rc_lowpass_2ins cpp jai
   test preamp_eq cpp jai
   test preamp_eq_comb cpp jai
   test hard_clipper cpp jai
   test diode_clipper cpp
   test diode_circuit cpp
   test simple_triode cpp jai
   test bassman_tone_stack cpp jai
   test baxandall_eq cpp jai
fi

if [[ "$*" = *bad_configs* ]]; then
    echo "Running bad config tests..."
    cd "${SCRIPT_DIR}/bad_configs"
    python3 run_tests.py
fi
