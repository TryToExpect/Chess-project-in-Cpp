cmake_minimum_required(VERSION 3.16)
project(SFML_CHESS)

set(CMAKE_CXX_STANDARD 17)

# 🔹 Ścieżka do SFML (zmień na swoją)
set(SFML_DIR "C:/SFML_3.0.2/build")

find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)

add_executable(SFML_CHESS main.cpp)

target_link_libraries(SFML_CHESS PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)