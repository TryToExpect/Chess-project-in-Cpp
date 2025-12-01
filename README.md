## How to build project

# Linux
sudo apt install libsfml-dev cmake g++
mkdir build && cd build
cmake ..
cmake --build .
./chess

# Windows
pacman -S mingw-w64-ucrt-x86_64-sfml mingw-w64-ucrt-x86_64-cmake
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./chess.exe

# MacOS
brew install sfml cmake
mkdir build && cd build
cmake ..
cmake --build .
./chess

