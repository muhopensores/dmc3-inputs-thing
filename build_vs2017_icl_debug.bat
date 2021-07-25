mkdir build86
cd build86
cmake .. -G "Visual Studio 15 2017" -T "Intel C++ Compiler 19.0" -DDEVELOPER_MODE=OFF
cmake --build . --config Debug
cd ..
pause