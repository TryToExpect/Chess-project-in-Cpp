#include <iostream>
#include <vector>
#include <cmath>
#include <string>

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
    bool hasMoved; // Track if piece has moved (essential for Castling and Pawn initial move)

    Piece(Color c, PieceType t) : color(c), type(t), hasMoved(false) {}
    virtual ~Piece() {}

    // Pure virtual function to get the character symbol of the piece
    virtual char getSymbol() const = 0;

    // Checks if the move is geometrically valid for the piece type.
    // DOES NOT check for obstructions (except for Pawn capture logic) or King safety.
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
public:
    Piece* grid[8][8];          // 8x8 Grid of pointers to Pieces
    Color turn;                 // Current player's turn

    // State for En Passant
    Move lastMove;
    bool lastMoveWasDoublePawnPush;

    Board() {
        turn = WHITE;
        lastMoveWasDoublePawnPush = false;
        // Initialize grid with nullptrs
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                grid[i][j] = nullptr;
        setup();
    }

    ~Board() {
        // Clean up memory
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                if(grid[i][j]) delete grid[i][j];
    }

    void setup() {
        // Setup Black Pieces (Rows 0, 1)
        grid[0][0] = new Rook(BLACK); grid[0][1] = new Knight(BLACK); grid[0][2] = new Bishop(BLACK);
        grid[0][3] = new Queen(BLACK); grid[0][4] = new King(BLACK);
        grid[0][5] = new Bishop(BLACK); grid[0][6] = new Knight(BLACK); grid[0][7] = new Rook(BLACK);
        for(int i=0; i<8; i++) grid[1][i] = new Pawn(BLACK);

        // Setup White Pieces (Rows 6, 7)
        grid[7][0] = new Rook(WHITE); grid[7][1] = new Knight(WHITE); grid[7][2] = new Bishop(WHITE);
        grid[7][3] = new Queen(WHITE); grid[7][4] = new King(WHITE);
        grid[7][5] = new Bishop(WHITE); grid[7][6] = new Knight(WHITE); grid[7][7] = new Rook(WHITE);
        for(int i=0; i<8; i++) grid[6][i] = new Pawn(WHITE);
    }

    // Checks if the path between two points is empty (for sliding pieces: Rook, Bishop, Queen)
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

    // Returns a pointer to the piece at (r, c) or nullptr if out of bounds
    Piece* getPiece(int r, int c) {
        if(r<0 || r>7 || c<0 || c>7) return nullptr;
        return grid[r][c];
    }

    // Checks if a specific square is under attack by the opponent
    bool isSquareAttacked(int r, int c, Color attackerColor) {
        for(int i=0; i<8; i++) {
            for(int j=0; j<8; j++) {
                Piece* p = grid[i][j];
                if(p && p->color == attackerColor) {
                    // Check if this enemy piece *could* move to (r,c)
                    if(p->isPseudoLegal(i, j, r, c, *this)) {
                         // For sliding pieces, we must ensure the path is clear to attack
                         if (p->type == ROOK || p->type == BISHOP || p->type == QUEEN) {
                             if(isPathClear(i, j, r, c)) return true;
                         } else {
                             // Knights, Kings, Pawns don't need isPathClear checks for attacks
                             return true;
                         }
                    }
                }
            }
        }
        return false;
    }

    // Locates the King of the given color
    void findKing(Color c, int &kr, int &kc) {
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                if(grid[i][j] && grid[i][j]->type == KING && grid[i][j]->color == c) {
                    kr = i; kc = j; return;
                }
    }

    // Checks if the current player's King is in Check
    bool isInCheck(Color c) {
        int kr, kc;
        findKing(c, kr, kc);
        Color enemy = (c == WHITE) ? BLACK : WHITE;
        return isSquareAttacked(kr, kc, enemy);
    }

    // CRITICAL: Simulates a move to see if it leaves the King in check.
    // If it does, the move is ILLEGAL.
    bool tryMove(Move m) {
        Piece* srcP = grid[m.r1][m.c1];
        Piece* destP = grid[m.r2][m.c2];

        // Handle En Passant Simulation
        Piece* enPassantVictim = nullptr;
        int epR = m.r1, epC = m.c2;

        if (m.isEnPassant) {
            enPassantVictim = grid[epR][epC];
            grid[epR][epC] = nullptr;
        }

        // Apply move temporarily
        grid[m.r2][m.c2] = srcP;
        grid[m.r1][m.c1] = nullptr;

        // Check if King is safe
        bool kingSafe = !isInCheck(srcP->color);

        // Revert move
        grid[m.r1][m.c1] = srcP;
        grid[m.r2][m.c2] = destP;
        if (m.isEnPassant) {
            grid[epR][epC] = enPassantVictim;
        }

        return kingSafe;
    }

    // Executes the move permanently
    void makeMove(Move m) {
        Piece* p = grid[m.r1][m.c1];

        // Handle actual En Passant Capture
        if (m.isEnPassant) {
            int epRow = m.r1;
            int epCol = m.c2;
            delete grid[epRow][epCol];
            grid[epRow][epCol] = nullptr;
            cout << "--- En Passant Capture! ---" << endl;
        }

        // Handle Castling (Moving the Rook)
        if (m.isCastling) {
            int row = m.r1;
            bool kingSide = (m.c2 > m.c1);
            int rookSrcCol = kingSide ? 7 : 0;
            int rookDestCol = kingSide ? 5 : 3;

            Piece* rook = grid[row][rookSrcCol];
            grid[row][rookDestCol] = rook;
            grid[row][rookSrcCol] = nullptr;
            if(rook) rook->hasMoved = true;
            cout << "--- Castling ---" << endl;
        }

        // Standard Capture
        if (grid[m.r2][m.c2] != nullptr) {
            delete grid[m.r2][m.c2];
        }

        // Move the piece
        grid[m.r2][m.c2] = p;
        grid[m.r1][m.c1] = nullptr;

        // Flag logic for En Passant in next turn
        if (p->type == PAWN && abs(m.r2 - m.r1) == 2) {
            lastMoveWasDoublePawnPush = true;
        } else {
            lastMoveWasDoublePawnPush = false;
        }

        // Pawn Promotion (Auto-Queen for simplicity)
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

    // Generates all possible moves to check for Mate/Stalemate
    bool hasLegalMoves(Color c) {
        for(int r1=0; r1<8; r1++) {
            for(int c1=0; c1<8; c1++) {
                Piece* p = grid[r1][c1];
                if(p && p->color == c) {
                    for(int r2=0; r2<8; r2++) {
                        for(int c2=0; c2<8; c2++) {
                            // First check geometry
                            if(p->isPseudoLegal(r1, c1, r2, c2, *this)) {
                                Move m = {r1, c1, r2, c2};

                                // Tag En Passant
                                if (p->type == PAWN && abs(c1 - c2) == 1 && grid[r2][c2] == nullptr) {
                                     m.isEnPassant = true;
                                }

                                // Check obstructions for sliders
                                if(p->type == ROOK || p->type == BISHOP || p->type == QUEEN) {
                                    if(!isPathClear(r1, c1, r2, c2)) continue;
                                }

                                // Simulate to check for Check
                                if(tryMove(m)) return true; // Found at least one legal move
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
            cout << 8 - i << " | "; // Rank numbers
            for (int j = 0; j < 8; j++) {
                if (grid[i][j] == nullptr) {
                    // Clean visual style: simple dots for empty squares
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

        // Game End Logic
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
    int direction = (color == WHITE) ? -1 : 1; // White moves up (-1), Black moves down (+1)
    int startRow = (color == WHITE) ? 6 : 1;

    // Move forward 1
    if (c1 == c2 && r2 == r1 + direction) {
        return board.getPiece(r2, c2) == nullptr;
    }
    // Move forward 2 (from start)
    if (c1 == c2 && r2 == r1 + 2 * direction && r1 == startRow) {
        return board.getPiece(r1 + direction, c1) == nullptr && board.getPiece(r2, c2) == nullptr;
    }
    // Diagonal Capture
    if (abs(c1 - c2) == 1 && r2 == r1 + direction) {
        Piece* target = board.getPiece(r2, c2);
        if (target != nullptr && target->color != color) return true;

        // EN PASSANT CHECK
        // Target is empty, but "Double Pawn Push" happened previously
        if (target == nullptr && board.lastMoveWasDoublePawnPush) {
            // Check if opponent's pawn ended up exactly next to us
            if (board.lastMove.r2 == r1 && board.lastMove.c2 == c2) {
                return true;
            }
        }
    }
    return false;
}

bool Rook::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    if (r1 != r2 && c1 != c2) return false; // Must be straight line
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false; // Cannot capture own piece
    return true; // Note: Path obstruction is checked in Board::isPathClear
}

bool Knight::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    int dr = abs(r1 - r2);
    int dc = abs(c1 - c2);
    // L-shape: 2x1 or 1x2
    if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false;
    return true;
}

bool Bishop::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    if (abs(r1 - r2) != abs(c1 - c2)) return false; // Must be diagonal
    Piece* p = board.getPiece(r2, c2);
    if (p != nullptr && p->color == color) return false;
    return true;
}

bool Queen::isPseudoLegal(int r1, int c1, int r2, int c2, Board& board) {
    // Combine Rook and Bishop logic
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

    // Standard move (1 square any direction)
    if (dr <= 1 && dc <= 1) {
        Piece* p = board.getPiece(r2, c2);
        if (p != nullptr && p->color == color) return false;
        return true;
    }

    // CASTLING Logic
    // Conditions: King moved 2 squares sideways, never moved before.
    if (dr == 0 && dc == 2 && !hasMoved) {
        // Cannot castle while in check
        if(board.isInCheck(color)) return false;

        int rookCol = (c2 > c1) ? 7 : 0; // Rook at H (7) or A (0)
        Piece* rook = board.getPiece(r1, rookCol);

        // Rook must exist and not have moved
        if (rook == nullptr || rook->type != ROOK || rook->hasMoved) return false;

        // Path between King and Rook must be empty
        if (!board.isPathClear(r1, c1, r1, rookCol)) return false;

        // The square the King crosses cannot be under attack
        int direction = (c2 > c1) ? 1 : -1;
        Color enemy = (color == WHITE) ? BLACK : WHITE;
        if (board.isSquareAttacked(r1, c1 + direction, enemy)) return false;

        return true;
    }

    return false;
}

// --- Main Loop ---
int main() {
    Board game;
    string startCoord, endCoord;

    cout << " C++ CHESS  " << endl;
    cout << "Enter moves like: e2 e4" << endl;
    cout << "Type 'exit' to quit." << endl << endl;

    while (true) {
        game.display();

        cout << "Enter move (source destination): ";
        cin >> startCoord;
        if (startCoord == "exit") break;
        cin >> endCoord;

        // Parse coordinates (e.g., "e2" -> col 4, row 6)
        int c1 = startCoord[0] - 'a';
        int r1 = 8 - (startCoord[1] - '0');
        int c2 = endCoord[0] - 'a';
        int r2 = 8 - (endCoord[1] - '0');

        // Bounds check
        if (r1 < 0 || r1 > 7 || c1 < 0 || c1 > 7 || r2 < 0 || r2 > 7 || c2 < 0 || c2 > 7) {
            cout << "Error: Invalid coordinates!" << endl;
            continue;
        }

        Piece* p = game.getPiece(r1, c1);

        // Basic ownership validation
        if (!p || p->color != game.turn) {
            cout << "Error: That is not your piece or the square is empty!" << endl;
            continue;
        }

        // 1. Check Geometry (Pseudo-Legal)
        if (!p->isPseudoLegal(r1, c1, r2, c2, game)) {
            cout << "Error: Invalid move for this piece type." << endl;
            continue;
        }

        // 2. Check Path Obstructions (for sliding pieces)
        if (p->type == ROOK || p->type == BISHOP || p->type == QUEEN) {
            if (!game.isPathClear(r1, c1, r2, c2)) {
                cout << "Error: Path is blocked!" << endl;
                continue;
            }
        }

        // 3. Construct Move object
        Move m = {r1, c1, r2, c2};

        // Detect En Passant scenario
        if (p->type == PAWN && abs(c1 - c2) == 1 && game.getPiece(r2, c2) == nullptr) {
            m.isEnPassant = true;
        }
        // Detect Castling scenario
        if (p->type == KING && abs(c1 - c2) == 2) {
            m.isCastling = true;
        }

        // 4. SIMULATION: Ensure move is fully legal (King safety)
        if (!game.tryMove(m)) {
            cout << "Illegal Move! Your King would be in check." << endl;
            continue;
        }

        // 5. Execute Move
        game.makeMove(m);
    }

    return 0;
}
