@echo off
set /p filename="Enter filename without extension like modName:"

call:DoReplace "ModSample" "%filename%" src\mods\ModSample.cpp src\mods\%filename%.cpp
call:DoReplace "ModSample" "%filename%" src\mods\ModSample.hpp src\mods\%filename%.hpp
echo Copied cmakelist addition
(
echo     mods/%filename%.hpp
echo     mods/%filename%.cpp
)| clip
notepad src\CMakeLists.txt
exit /b

:DoReplace
echo ^(Get-Content "%3"^) ^| ForEach-Object { $_ -replace %1, %2 } ^| Set-Content %4>Rep.ps1
Powershell.exe -executionpolicy ByPass -File Rep.ps1
if exist Rep.ps1 del Rep.ps1
