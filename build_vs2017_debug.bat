mkdir build86
cd build86
REM CLANG LINE
REM cmake .. -G "Visual Studio 15 2017" -T LLVM_v141 -DDEVELOPER_MODE=OFF
REM ICL LINE
REM cmake .. -G "Visual Studio 15 2017" -T "Intel C++ Compiler 19.0" -DDEVELOPER_MODE=OFF
cmake .. -G "Visual Studio 15 2017" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDEVELOPER_MODE=OFF
cmake --build . --config Debug
cd ..
REM emacs lsp bullshit
REM mkdir lsp
REM cd lsp
REM cmake .. -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER="clang-cl.exe" -DCMAKE_CXX_COMPILER="clang-cl.exe" -DCMAKE_LINKER="lld-link.exe"
REM copy compile_commands.json ..\compile_commands.json
REM cd ..
pause