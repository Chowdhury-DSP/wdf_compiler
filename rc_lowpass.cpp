#include "rc_lowpass.h"

#include <iostream>

int main()
{
    std::cout << "RC Lowpass test\n";

    static constexpr float fs = 48000.0f;
    Impedances impedances {};
    State state {};

    calc_impedances (impedances, fs);
    const auto output = process (state, impedances, 1.0f);

    std::cout << output << '\n';

    return 0;
}
