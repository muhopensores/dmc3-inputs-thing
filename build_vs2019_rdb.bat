mkdir build86
cd build86
REM CLANG LINE
REM cmake .. -G "Visual Studio 16 2019" -T ClangCl -DDEVELOPER_MODE=OFF
cmake .. -G "Visual Studio 16 2019" -A x64 -T ClangCl -DDEVELOPER_MODE=OFF
cmake --build . --config RelWithDebInfo
cd ..
pause