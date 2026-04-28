@echo off
setlocal

if not exist build86_shipping mkdir build86_shipping
cd build86_shipping

cmake .. -G "Visual Studio 17 2022" -A "Win32" -DBUILD_SHIPPING=ON
cmake --build . --config Release
cd ..
PAUSE
