#!/bin/bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [[ -n "$1" ]]; then
    cpp_compiler="$1"
else
    cpp_compiler="clang"
fi


if [ ! -d "${SCRIPT_DIR}/libs/armadillo" ]; then
    git clone https://gitlab.com/conradsnicta/armadillo-code "${SCRIPT_DIR}/libs/armadillo"
fi
if [ ! -d "${SCRIPT_DIR}/libs/rt-wdf" ]; then
    git clone https://github.com/RT-WDF/rt-wdf_lib "${SCRIPT_DIR}/libs/rt-wdf"
fi

flags="--std=c++20 -O3 -I${SCRIPT_DIR}/../xsimd/include -I${SCRIPT_DIR}/../../lib -Ilibs/rt-wdf/Libs -Ilibs/armadillo/include"
if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
    flags="${flags} -I$HOME/Code/Faust/include"
else
    flags="${flags} -lstdc++ -I/opt/homebrew/include"
fi

cpp_test () {
    test="$1"
    echo "Running ${test}"
    clang ${test}.cpp ${flags} -o ${test}.exe
    ./${test}.exe
}

viola_test () {
    circuit="$1"
    test="${circuit}_viola"
    echo "Running ${test}"
    clang ${test}.cpp viola/${circuit}/*.cpp -Iviola -Iviola/${circuit} ${flags} -o ${test}.exe
    ./${test}.exe
}

ciaramella_test () {
    circuit="$1"
    test="${circuit}_ciaramella"
    externals=""
    if [ -f "ciaramella/${circuit}/externals.h" ]; then
        externals="-include ciaramella/${circuit}/externals.h"
    fi
    echo "Running ${test}"
    clang ${test}.cpp ciaramella/${circuit}/generated/*.cpp -Iciaramella/${circuit}/generated -I${SCRIPT_DIR}/../baxandall_eq ${externals} ${flags} -o ${test}.exe
    ./${test}.exe
}

# cpp_test rc_lowpass_chowdsp_poly
# cpp_test preamp_eq_comb_chowdsp_poly
# cpp_test diode_clipper_chowdsp_poly
# cpp_test simple_triode_chowdsp_poly
# cpp_test baxandall_eq_chowdsp_poly
# cpp_test reductions_circuit_chowdsp_poly

cpp_test rc_lowpass_rt_wdf
cpp_test preamp_eq_rt_wdf
cpp_test diode_clipper_rt_wdf
cpp_test simple_triode_rt_wdf
cpp_test baxandall_eq_rt_wdf
cpp_test reductions_circuit_rt_wdf

cpp_test rc_lowpass_faust
cpp_test rc_lowpass_double_faust
cpp_test preamp_eq_faust
cpp_test diode_clipper_faust
cpp_test baxandall_eq_faust
cpp_test reductions_circuit_faust

viola_test rc_lowpass
viola_test preamp_eq
viola_test diode_clipper
viola_test baxandall_eq
viola_test reductions_circuit

ciaramella_test rc_lowpass
ciaramella_test preamp_eq
ciaramella_test diode_clipper
ciaramella_test baxandall_eq
ciaramella_test reductions_circuit
