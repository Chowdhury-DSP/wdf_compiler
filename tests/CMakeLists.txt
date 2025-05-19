cmake_minimum_required(VERSION 3.20)
project(wdf_compiler_tests VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 20)

set(WDF_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/../compiler/wdf_compiler)

add_custom_command(
    OUTPUT rc_lowpass/rc_lowpass.h
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling rc_lowpass WDF"
    COMMAND ${WDF_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/rc_lowpass/rc_lowpass.wdf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rc_lowpass/rc_lowpass.wdf
)
add_executable(rc_lowpass
    rc_lowpass/rc_lowpass.h
    rc_lowpass/rc_lowpass.cpp
)
