@echo off
set /p filename="Enter filename without extension like modName:"
copy src\mods\ModSample.cpp src\mods\%filename%.cpp
copy src\mods\ModSample.hpp src\mods\%filename%.hpp
(
echo     mods/%filename%.hpp
echo     mods/%filename%.cpp
)| clip
notepad src\CMakeLists.txt