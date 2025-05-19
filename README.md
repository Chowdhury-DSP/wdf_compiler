

```bash
jai compiler/wdf_compiler.jai && ./compiler/wdf_compiler rc_lowpass.wdf
clang rc_lowpass.cpp --std=c++20 -lstdc++ -o rc_lowpass.exe && ./rc_lowpass.exe
```
