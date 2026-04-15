REM emacs lsp bullshit
del lsp /Q
mkdir lsp
cd lsp
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER_ARG1="-target, x86-pc-windows-msvc, --driver-mode=cl, -fmsc-version=1930 -m32" -DCMAKE_CXX_COMPILER_ARG1="-target, x86-pc-windows-msvc, --driver-mode=cl, -fmsc-version=1930 -m32" -DCMAKE_MAKE_PROGRAM="make" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
copy compile_commands.json ..\compile_commands.json
cd ..
PAUSE
