## How to 
Firstly, clone this repository by executing following command:
git clone https://github.com/TryToExpect/Chess-project-in-Cpp.git
## How to build project

# Linux
sudo apt update
sudo apt install libsfml-dev cmake g++
mkdir build && cd build
cmake ..
cmake --build .
./SFML_CHESS

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

