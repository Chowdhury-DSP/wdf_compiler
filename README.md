
RC Lowpass Example:
```bash
jai compiler/wdf_compiler.jai && ./compiler/wdf_compiler tests/rc_lowpass/rc_lowpass.wdf tests/rc_lowpass/rc_lowpass.h
cd tests/rc_lowpass
clang rc_lowpass.cpp --std=c++20 -lstdc++ -o rc_lowpass.exe && ./rc_lowpass.exe
```

RL Lowpass Example:
```bash
jai compiler/wdf_compiler.jai && ./compiler/wdf_compiler tests/rl_lowpass/rl_lowpass.wdf tests/rl_lowpass/rl_lowpass.h
cd tests/rl_lowpass
clang rl_lowpass.cpp --std=c++20 -lstdc++ -o rl_lowpass.exe && ./rl_lowpass.exe
```
