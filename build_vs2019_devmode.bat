git pull --recurse-submodules
git submodule update --init --recursive
mkdir build86
cd build86
cmake .. -G "Visual Studio 16 2019" -DDEVELOPER_MODE=ON
cmake --build . --config Release