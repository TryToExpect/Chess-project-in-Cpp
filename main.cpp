#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>

using namespace std;

enum PieceType { NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };
enum Color { WHITE, BLACK };

struct Piece {
    PieceType type;
    Color color;
};

array<array<Piece, 8>, 8> board; // Board 8x8

void setupBoard() {
    // Clean board
    for (auto &row : board)
        for (auto &cell : row)
            cell.type = NONE;

    // White
    for (int i = 0; i < 8; i++) board[1][i] = { PAWN, WHITE };
    board[0][0] = { ROOK, WHITE }; board[0][7] = { ROOK, WHITE };
    board[0][1] = { KNIGHT, WHITE }; board[0][6] = { KNIGHT, WHITE };
    board[0][2] = { BISHOP, WHITE }; board[0][5] = { BISHOP, WHITE };
    board[0][3] = { QUEEN, WHITE };  board[0][4] = { KING, WHITE };

    // Black
    for (int i = 0; i < 8; i++) board[6][i] = { PAWN, BLACK };
    board[7][0] = { ROOK, BLACK }; board[7][7] = { ROOK, BLACK };
    board[7][1] = { KNIGHT, BLACK }; board[7][6] = { KNIGHT, BLACK };
    board[7][2] = { BISHOP, BLACK }; board[7][5] = { BISHOP, BLACK };
    board[7][3] = { QUEEN, BLACK };  board[7][4] = { KING, BLACK };
}

int main() {
    sf::RenderWindow window({512, 512}, "SFML Chess", sf::Style::Close);
    window.setFramerateLimit(60);

    // === LOADING TEXTURES ===
    sf::Texture boardTexture;
    if (!boardTexture.loadFromFile("images/board.png")) {
        cerr << "Nie udalo sie zaladowac board.png\n";
        return -1;
    }

    array<sf::Texture, 12> textures;
    string pieceFiles[12] = {
        "images/wp.png", "images/wr.png", "images/wn.png", "images/wb.png", "images/wq.png", "images/wk.png",
        "images/bp.png", "images/br.png", "images/bn.png", "images/bb.png", "images/bq.png", "images/bk.png"
    };

    for (int i = 0; i < 12; ++i) {
        if (!textures[i].loadFromFile(pieceFiles[i])) {
            cerr << "Blad wczytywania: " << pieceFiles[i] << endl;
            return -1;
        }
    }

    // === Setting board ===
    setupBoard();
    sf::Sprite boardSprite(boardTexture);

    // === GAME MAIN LOOP ===
    while (window.isOpen()) {
        for (auto event = sf::Event{}; window.pollEvent(event);) {
            if (event.is<sf::Event::Closed>())
                window.close();

            // Mouse clicking - testing
            if (event.is<sf::Event::MouseButtonPressed>()) {
                auto mouse = event.getIf<sf::Event::MouseButtonPressed>();
                int x = mouse->x / 64;
                int y = 7 - (mouse->y / 64);
                cout << "Kliknięto: " << char('a' + x) << (y + 1) << endl;
            }
        }

        window.clear();
        window.draw(boardSprite);

        // Drawing figures
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece p = board[i][j];
                if (p.type != NONE) {
                    sf::Sprite pieceSprite;
                    int index = 0;
                    if (p.color == WHITE)
                        index = p.type - 1;
                    else
                        index = 6 + (p.type - 1);

                    pieceSprite.setTexture(textures[index]);
                    pieceSprite.setPosition(j * 64, (7 - i) * 64);
                    window.draw(pieceSprite);
                }
            }
        }

        window.display();
    }

    return 0;
}

