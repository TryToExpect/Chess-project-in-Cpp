#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>

using namespace std;

// Helper Definitions
enum Color { WHITE, BLACK, NONE };
enum PieceType { KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN, EMPTY };

// Structure to represent a move
struct Move {
    int r1, c1, r2, c2;         // Source and Destination coordinates
    bool isEnPassant = false;   // Special move flag: En Passant
    bool isCastling = false;    // Special move flag: Castling
    bool isPromotion = false;   // Special move flag: Pawn Promotion
    PieceType promotionPiece = QUEEN; // Default promotion is Queen
};

// Forward declaration
class Board;

// --- Abstract Base Class for Pieces ---
class Piece {
public:
    Color color;
    PieceType type;
    bool hasMoved;

    Piece(Color c, PieceType t) : color(c), type(t), hasMoved(false) {}
    virtual ~Piece() {}

    virtual char getSymbol() const = 0;
    virtual bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) = 0;
};

// --- Piece Implementations ---

class Pawn : public Piece {
public:
    Pawn(Color c) : Piece(c, PAWN) {}
    char getSymbol() const override { return (color == WHITE) ? 'P' : 'p'; }
    bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) override;
};

class Rook : public Piece {
public:
    Rook(Color c) : Piece(c, ROOK) {}
    char getSymbol() const override { return (color == WHITE) ? 'R' : 'r'; }
    bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) override;
};

class Knight : public Piece {
public:
    Knight(Color c) : Piece(c, KNIGHT) {}
    char getSymbol() const override { return (color == WHITE) ? 'N' : 'n'; }
    bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) override;
};

class Bishop : public Piece {
public:
    Bishop(Color c) : Piece(c, BISHOP) {}
    char getSymbol() const override { return (color == WHITE) ? 'B' : 'b'; }
    bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) override;
};

class Queen : public Piece {
public:
    Queen(Color c) : Piece(c, QUEEN) {}
    char getSymbol() const override { return (color == WHITE) ? 'Q' : 'q'; }
    bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) override;
};

class King : public Piece {
public:
    King(Color c) : Piece(c, KING) {}
    char getSymbol() const override { return (color == WHITE) ? 'K' : 'k'; }
    bool isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) override;
};

// --- Board Class (Main Game Logic) ---
class Board {
private:
    // Helper to create pieces dynamically
    Piece* createPiece(PieceType type, Color color) {
        switch (type) {
            case ROOK: return new Rook(color);
            case KNIGHT: return new Knight(color);
            case BISHOP: return new Bishop(color);
            case QUEEN: return new Queen(color);
            case KING: return new King(color);
            default: return nullptr;
        }
    }

public:
    Piece* grid[8][8];          // 8x8 Grid of pointers to Pieces
    Color turn;                 // Current player's turn

    // State for En Passant
    Move lastMove;
    bool lastMoveWasDoublePawnPush;

    // Zmieniony konstruktor przyjmujący tryb gry
    Board(bool isFisher960) {
        turn = WHITE;
        lastMoveWasDoublePawnPush = false;
        // Initialize grid with nullptrs
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                grid[i][j] = nullptr;

        setup(isFisher960);
    }

    ~Board() {
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                if(grid[i][j]) delete grid[i][j];
    }

    // Nowa funkcja setup obsługująca oba tryby
    void setup(bool isFisher960) {
        PieceType layout[8];
        std::srand(std::time(0));

        if (!isFisher960) {
            // --- KLASYCZNE ---
            layout[0] = ROOK; layout[1] = KNIGHT; layout[2] = BISHOP; layout[3] = QUEEN;
            layout[4] = KING; layout[5] = BISHOP; layout[6] = KNIGHT; layout[7] = ROOK;
        }
        else {
            // --- FISHER RANDOM (CHESS960) ---
            std::vector<int> emptySlots = {0, 1, 2, 3, 4, 5, 6, 7};
            std::vector<int> boardVec(8, -1);

            // 1. Gońce (różne kolory pól)
            std::vector<int> evenSlots = {0, 2, 4, 6};
            std::vector<int> oddSlots = {1, 3, 5, 7};
            int posB1 = evenSlots[rand() % evenSlots.size()];
            int posB2 = oddSlots[rand() % oddSlots.size()];
            boardVec[posB1] = BISHOP;
            boardVec[posB2] = BISHOP;

            emptySlots.erase(std::remove(emptySlots.begin(), emptySlots.end(), posB1), emptySlots.end());
            emptySlots.erase(std::remove(emptySlots.begin(), emptySlots.end(), posB2), emptySlots.end());

            // 2. Hetman
            int idxQ = rand() % emptySlots.size();
            boardVec[emptySlots[idxQ]] = QUEEN;
            emptySlots.erase(emptySlots.begin() + idxQ);

            // 3. Skoczki
            for (int k = 0; k < 2; k++) {
                int idxN = rand() % emptySlots.size();
                boardVec[emptySlots[idxN]] = KNIGHT;
                emptySlots.erase(emptySlots.begin() + idxN);
            }

            // 4. Wieża, Król, Wieża (w pozostałych slotach, w tej kolejności)
            boardVec[emptySlots[0]] = ROOK;
            boardVec[emptySlots[1]] = KING;
            boardVec[emptySlots[2]] = ROOK;

            for(int i=0; i<8; i++) layout[i] = (PieceType)boardVec[i];
        }

        // Ustawienie figur na planszy
        for(int i=0; i<8; i++) {
            grid[0][i] = createPiece(layout[i], BLACK); // Wiersz 0
            grid[1][i] = new Pawn(BLACK);               // Wiersz 1

            grid[7][i] = createPiece(layout[i], WHITE); // Wiersz 7
            grid[6][i] = new Pawn(WHITE);               // Wiersz 6
        }
    }

    bool isPathClear(int r1, int c1, int r2, int c2) {
        int dr = (r2 - r1);
        int dc = (c2 - c1);
        int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);

        int r = r1 + stepR;
        int c = c1 + stepC;

        while (r != r2 || c != c2) {
            if (grid[r][c] != nullptr) return false;
            r += stepR;
            c += stepC;
        }
        return true;
    }

    Piece* getPiece(int r, int c) {
        if(r<0 || r>7 || c<0 || c>7) return nullptr;
        return grid[r][c];
    }

    bool isSquareAttacked(int r, int c, Color attackerColor) {
        for(int i=0; i<8; i++) {
            for(int j=0; j<8; j++) {
                Piece* p = grid[i][j];
                if(p && p->color == attackerColor) {
                    if(p->isPseudoLegal(i, j, r, c, *this)) {
                         if (p->type == ROOK || p->type == BISHOP || p->type == QUEEN) {
                             if(isPathClear(i, j, r, c)) return true;
                         } else {
                             return true;
                         }
                    }
                }
            }
        }
        return false;
    }

    void findKing(Color c, int &kr, int &kc) {
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                if(grid[i][j] && grid[i][j]->type == KING && grid[i][j]->color == c) {
                    kr = i; kc = j; return;
                }
    }

    bool isInCheck(Color c) {
        int kr, kc;
        findKing(c, kr, kc);
        Color enemy = (c == WHITE) ? BLACK : WHITE;
        return isSquareAttacked(kr, kc, enemy);
    }

    bool tryMove(Move m) {
        Piece* srcP = grid[m.r1][m.c1];
        Piece* destP = grid[m.r2][m.c2];

        Piece* enPassantVictim = nullptr;
        int epR = m.r1, epC = m.c2;

        if (m.isEnPassant) {
            enPassantVictim = grid[epR][epC];
            grid[epR][epC] = nullptr;
        }

        grid[m.r2][m.c2] = srcP;
        grid[m.r1][m.c1] = nullptr;

        bool kingSafe = !isInCheck(srcP->color);

        grid[m.r1][m.c1] = srcP;
        grid[m.r2][m.c2] = destP;
        if (m.isEnPassant) {
            grid[epR][epC] = enPassantVictim;
        }

        return kingSafe;
    }

    void makeMove(Move m) {
        Piece* p = grid[m.r1][m.c1];

        if (m.isEnPassant) {
            int epRow = m.r1;
            int epCol = m.c2;
            delete grid[epRow][epCol];
            grid[epRow][epCol] = nullptr;
            cout << "--- En Passant Capture! ---" << endl;
        }

        if (m.isCastling) {
            int row = m.r1;
            bool kingSide = (m.c2 > m.c1); // Logic adjusted for standard input intent
            // In Fisher, we calculate target rook col

            // Logic to find THE specific rook for this castling
             // Re-calculate rook col (same logic as King::isPseudoLegal)
            int direction = (m.c2 > m.c1) ? 1 : -1;
            int rookSrcCol = -1;
            for (int k = m.c1 + direction; k >= 0 && k <= 7; k += direction) {
                if (grid[row][k] != nullptr && grid[row][k]->type == ROOK) {
                    rookSrcCol = k; break;
                }
            }

            int rookDestCol = kingSide ? 5 : 3; // F or D

            if(rookSrcCol != -1) {
                Piece* rook = grid[row][rookSrcCol];
                grid[row][rookSrcCol] = nullptr; // Pick up rook
                grid[row][rookDestCol] = rook;   // Place rook
                rook->hasMoved = true;
            }
            cout << "--- Castling ---" << endl;
        }

        if (grid[m.r2][m.c2] != nullptr) {
            delete grid[m.r2][m.c2];
        }

        grid[m.r2][m.c2] = p;
        grid[m.r1][m.c1] = nullptr;

        if (p->type == PAWN && abs(m.r2 - m.r1) == 2) {
            lastMoveWasDoublePawnPush = true;
        } else {
            lastMoveWasDoublePawnPush = false;
        }

        if (p->type == PAWN && (m.r2 == 0 || m.r2 == 7)) {
            cout << "PROMOTION! Pawn promoted to Queen." << endl;
            delete p;
            grid[m.r2][m.c2] = new Queen(turn);
            p = grid[m.r2][m.c2];
        }

        p->hasMoved = true;
        lastMove = m;
        turn = (turn == WHITE) ? BLACK : WHITE;
    }

    bool hasLegalMoves(Color c) {
        for(int r1=0; r1<8; r1++) {
            for(int c1=0; c1<8; c1++) {
                Piece* p = grid[r1][c1];
                if(p && p->color == c) {
                    for(int r2=0; r2<8; r2++) {
                        for(int c2=0; c2<8; c2++) {
                            if(p->isPseudoLegal(r1, c1, r2, c2, *this)) {
                                Move m = {r1, c1, r2, c2};
                                if (p->type == PAWN && abs(c1 - c2) == 1 && grid[r2][c2] == nullptr) {
                                     m.isEnPassant = true;
                                }
                                if(p->type == ROOK || p->type == BISHOP || p->type == QUEEN) {
                                    if(!isPathClear(r1, c1, r2, c2)) continue;
                                }
                                if(tryMove(m)) return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    void display() {
        cout << "\n    a b c d e f g h\n";
        cout << "  +-----------------+\n";
        for (int i = 0; i < 8; i++) {
            cout << 8 - i << " | ";
            for (int j = 0; j < 8; j++) {
                if (grid[i][j] == nullptr) {
                    cout << ".";
                } else {
                    cout << grid[i][j]->getSymbol();
                }
                cout << " ";
            }
            cout << "| " << 8 - i << endl;
        }
        cout << "  +-----------------+\n";
        cout << "    a b c d e f g h\n\n";

        if (isInCheck(turn)) {
            if (!hasLegalMoves(turn)) {
                cout << "!!! CHECKMATE !!! Winner: " << (turn == WHITE ? "BLACK" : "WHITE") << endl;
                exit(0);
            }
            cout << "!!! CHECK !!!" << endl;
        } else {
            if (!hasLegalMoves(turn)) {
                cout << "!!! STALEMATE (DRAW) !!!" << endl;
                exit(0);
            }
        }
        cout << "Turn: " << (turn == WHITE ? "WHITE (Uppercase)" : "BLACK (lowercase)") << endl;
    }
};

// --- Method Definitions (Logic) ---

bool Pawn::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    int direction = (color == WHITE) ? -1 : 1;
    int startRow = (color == WHITE) ? 6 : 1;

    if (c1 == c2 && r2 == r1 + direction) {
        return board.getPiece(r2, c2) == nullptr;
    }
    if (c1 == c2 && r2 == r1 + 2 * direction && r1 == startRow) {
        return board.getPiece(r1 + direction, c1) == nullptr && board.getPiece(r2, c2) == nullptr;
    }
    if (abs(c1 - c2) == 1 && r2 == r1 + direction) {
        Piece* target = board.getPiece(r2, c2);
        if (target != nullptr && target->color != color) return true;
        if (target == nullptr && board.lastMoveWasDoublePawnPush) {
            if (board.lastMove.r2 == r1 && board.lastMove.c2 == c2) {
                return true;
            }
        }
    }
    return false;
}

bool Rook::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    if (r1 != r2 && c1 != c2) return false;
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false;
    return true;
}

bool Knight::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    int dr = abs(r1 - r2);
    int dc = abs(c1 - c2);
    if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false;
    return true;
}

bool Bishop::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    if (abs(r1 - r2) != abs(c1 - c2)) return false;
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false;
    return true;
}

bool Queen::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    bool straight = (r1 == r2 || c1 == c2);
    bool diagonal = (abs(r1 - r2) == abs(c1 - c2));
    if (!straight && !diagonal) return false;
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false;
    return true;
}

bool King::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    int dr = abs(r1 - r2);
    int dc = abs(c1 - c2);

    // Standard move
    if (dr <= 1 && dc <= 1) {
        Piece* p = board.getPiece(r2, c2);
        if (p != nullptr && p->color == color) return false;
        return true;
    }

    // --- FISHER / CASTLING Logic ---
    if (dr == 0 && dc == 2 && !hasMoved) {
        if(board.isInCheck(color)) return false;

        bool isKingSide = (c2 > c1);
        int searchDir = isKingSide ? 1 : -1;
        int rookCol = -1;

        // Find rook
        for (int k = c1 + searchDir; k >= 0 && k <= 7; k += searchDir) {
            Piece* p = board.getPiece(r1, k);
            if (p != nullptr && p->type == ROOK && p->color == color) {
                rookCol = k;
                break;
            }
        }

        if (rookCol == -1) return false;

        Piece* rook = board.getPiece(r1, rookCol);
        if (rook == nullptr || rook->hasMoved) return false;

        int targetKingCol = isKingSide ? 6 : 2; // G or C
        int targetRookCol = isKingSide ? 5 : 3; // F or D

        // Path Clear Logic for King and Rook combined
        int minK = std::min(c1, targetKingCol);
        int maxK = std::max(c1, targetKingCol);
        int minR = std::min(rookCol, targetRookCol);
        int maxR = std::max(rookCol, targetRookCol);

        // Check King path for obstacles
        for (int k = minK; k <= maxK; k++) {
            Piece* p = board.getPiece(r1, k);
            if (p != nullptr && p != this && p != rook) return false;
        }
        // Check Rook path for obstacles
        for (int k = minR; k <= maxR; k++) {
            Piece* p = board.getPiece(r1, k);
            if (p != nullptr && p != this && p != rook) return false;
        }

        // Check if King passes through check
        Color enemy = (color == WHITE) ? BLACK : WHITE;
        int checkDir = (targetKingCol > c1) ? 1 : -1;
        int currentCheck = c1 + checkDir;

        while (true) {
            if (board.isSquareAttacked(r1, currentCheck, enemy)) return false;
            if (currentCheck == targetKingCol) break;
            currentCheck += checkDir;
        }

        return true;
    }

    return false;
}

// --- Main Loop ---
int main() {
    int choice;
    cout << "==========================" << endl;
    cout << "      C++ CHESS ENGINE    " << endl;
    cout << "==========================" << endl;
    cout << "Select Game Mode:" << endl;
    cout << "1. Standard Chess" << endl;
    cout << "2. Fisher Random (Chess960)" << endl;
    cout << "Choice: ";
    cin >> choice;

    bool fisherMode = (choice == 2);
    Board game(fisherMode);

    string startCoord, endCoord;

    cout << "\nInstructions:" << endl;
    cout << "- Enter moves as 'e2 e4' (start square, end square)" << endl;
    cout << "- To CASTLE: Move King 2 squares to desired side (e.g., e1 g1)" << endl;
    cout << "- Type 'exit' to quit." << endl << endl;

    while (true) {
        game.display();

        cout << "Enter move (source destination): ";
        cin >> startCoord;
        if (startCoord == "exit") break;
        cin >> endCoord;

        int c1 = startCoord[0] - 'a';
        int r1 = 8 - (startCoord[1] - '0');
        int c2 = endCoord[0] - 'a';
        int r2 = 8 - (endCoord[1] - '0');

        if (r1 < 0 || r1 > 7 || c1 < 0 || c1 > 7 || r2 < 0 || r2 > 7 || c2 < 0 || c2 > 7) {
            cout << "Error: Invalid coordinates!" << endl;
            continue;
        }

        Piece* p = game.getPiece(r1, c1);

        if (!p || p->color != game.turn) {
            cout << "Error: That is not your piece or the square is empty!" << endl;
            continue;
        }

        if (!p->isPseudoLegal(r1, c1, r2, c2, game)) {
            cout << "Error: Invalid move for this piece type." << endl;
            continue;
        }

        // Obstruction check for sliders
        if (p->type == ROOK || p->type == BISHOP || p->type == QUEEN) {
            if (!game.isPathClear(r1, c1, r2, c2)) {
                cout << "Error: Path is blocked!" << endl;
                continue;
            }
        }

        Move m = {r1, c1, r2, c2};

        if (p->type == PAWN && abs(c1 - c2) == 1 && game.getPiece(r2, c2) == nullptr) {
             m.isEnPassant = true;
        }
        if (p->type == KING && abs(c1 - c2) == 2) {
            m.isCastling = true;
        }

        if (!game.tryMove(m)) {
            cout << "Illegal Move! Your King would be in check." << endl;
            continue;
        }

        game.makeMove(m);
    }

    return 0;
}