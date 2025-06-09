
RC Lowpass Example:
```bash
cd tests/rc_lowpass
jai ../../compiler/wdf_compiler.jai && ../../compiler/wdf_compiler rc_lowpass.wdf rc_lowpass.h
clang rc_lowpass.cpp --std=c++20 -lstdc++ -o rc_lowpass.exe && ./rc_lowpass.exe
```

RL Lowpass Example:
```bash
cd tests/rl_lowpass
jai ../../compiler/wdf_compiler.jai && ../../compiler/wdf_compiler rl_lowpass.wdf rl_lowpass.h
clang rl_lowpass.cpp --std=c++20 -lstdc++ -o rl_lowpass.exe && ./rl_lowpass.exe
```

RC Bandpass Example:
```bash
cd tests/rc_bandpass
jai ../../compiler/wdf_compiler.jai && ../../compiler/wdf_compiler rc_bandpass.wdf rc_bandpass.h
clang rc_bandpass.cpp --std=c++20 -lstdc++ -o rc_bandpass.exe && ./rc_bandpass.exe
```

```bash
jai ../../compiler/wdf_compiler.jai && ../../compiler/wdf_compiler rc_lowpass_var.wdf rc_lowpass_var.h
clang rc_lowpass_var.cpp --std=c++20 -lstdc++ -o rc_lowpass_var.exe && ./rc_lowpass_var.exe
```
