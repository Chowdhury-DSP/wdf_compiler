cmake_minimum_required(VERSION 3.20)
project(wdf_compiler_tests VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 20)

set(WDF_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/../compiler/wdf_compiler)

add_custom_command(
    OUTPUT rc_lowpass/rc_lowpass.h
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling rc_lowpass WDF"
    COMMAND ${WDF_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/rc_lowpass/rc_lowpass.wdf ${CMAKE_CURRENT_SOURCE_DIR}/rc_lowpass/rc_lowpass.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rc_lowpass/rc_lowpass.wdf
)
add_executable(rc_lowpass
    rc_lowpass/rc_lowpass.h
    rc_lowpass/rc_lowpass.cpp
)

add_custom_command(
    OUTPUT rl_lowpass/rl_lowpass.h
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling rl_lowpass WDF"
    COMMAND ${WDF_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/rl_lowpass/rl_lowpass.wdf ${CMAKE_CURRENT_SOURCE_DIR}/rl_lowpass/rl_lowpass.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rl_lowpass/rl_lowpass.wdf
)
add_executable(rl_lowpass
    rl_lowpass/rl_lowpass.h
    rl_lowpass/rl_lowpass.cpp
)

add_custom_command(
    OUTPUT rc_bandpass/rc_bandpass.h
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling rc_bandpass WDF"
    COMMAND ${WDF_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/rc_bandpass/rc_bandpass.wdf ${CMAKE_CURRENT_SOURCE_DIR}/rc_bandpass/rc_bandpass.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rc_bandpass/rc_bandpass.wdf
)
add_executable(rc_bandpass
    rc_bandpass/rc_bandpass.h
    rc_bandpass/rc_bandpass.cpp
)
