@echo off
rem Builds elm.exe with MinGW g++ (install from https://winlibs.com or MSYS2), then runs it.
g++ -std=c++17 -O2 -Iinclude -o elm.exe src\*.cpp
if errorlevel 1 (echo Build failed & exit /b 1)
echo Built elm.exe - run it from this folder so Data\ is found.
elm.exe
