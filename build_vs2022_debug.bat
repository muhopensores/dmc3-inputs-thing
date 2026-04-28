mkdir build86
cd build86
cmake .. -G "Visual Studio 17 2022" -A "Win32" -DBUILD_SHIPPING=OFF
cmake --build . --config Debug
cd ..
PAUSE