@echo off
rmdir /s /q build 2>nul
cmake -G "MinGW Makefiles" -DENABLE_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build
