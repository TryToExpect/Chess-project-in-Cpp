cmake_minimum_required(VERSION 3.16)
project(SFML_CHESS)

set(CMAKE_CXX_STANDARD 17)

# On Linux you can usually install SFML 3 via your package manager.
# If you have SFML in a custom location, set SFML_DIR before configuring.
# Example (bash):
#   cmake -DSFML_DIR=/path/to/sfml/share/SFML ..

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

# Build sources in src/
add_executable(SFML_CHESS
	src/main.cpp
	src/Board.cpp
	src/Piece.cpp
	src/PieceManager.cpp
	src/GameLogic.cpp
	src/GameRecorder.cpp
	src/SoundManager.cpp
	src/Pieces/Rook.cpp
	src/Pieces/Pawn.cpp
	src/Pieces/Knight.cpp
	src/Pieces/Bishop.cpp
	src/Pieces/Queen.cpp
	src/Pieces/King.cpp
)

target_link_libraries(SFML_CHESS PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)