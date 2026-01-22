#include "GameLogic.hpp"
#include "GameRecorder.hpp"
#include "Pieces/Pawn.hpp"
#include "Pieces/Rook.hpp"
#include "Pieces/Knight.hpp"
#include "Pieces/Bishop.hpp"
#include "Pieces/Queen.hpp"
#include "Pieces/King.hpp"
#include <iostream>
#include <cmath>

GameLogic::GameLogic() : turn(Color::WHITE), lastMoveWasDoublePawnPush(false) {
    setup();
}

void GameLogic::setup() {
    // Clear grid
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            grid[i][j] = nullptr;
        }
    }

    // Setup Black Pieces (Rows 0, 1)
    grid[0][0] = std::make_unique<Rook>(Color::BLACK);
    grid[0][1] = std::make_unique<Knight>(Color::BLACK);
    grid[0][2] = std::make_unique<Bishop>(Color::BLACK);
    grid[0][3] = std::make_unique<Queen>(Color::BLACK);
    grid[0][4] = std::make_unique<King>(Color::BLACK);
    grid[0][5] = std::make_unique<Bishop>(Color::BLACK);
    grid[0][6] = std::make_unique<Knight>(Color::BLACK);
    grid[0][7] = std::make_unique<Rook>(Color::BLACK);

    for (int i = 0; i < 8; i++) {
        grid[1][i] = std::make_unique<Pawn>(Color::BLACK);
    }

    // Setup White Pieces (Rows 6, 7)
    for (int i = 0; i < 8; i++) {
        grid[6][i] = std::make_unique<Pawn>(Color::WHITE);
    }

    grid[7][0] = std::make_unique<Rook>(Color::WHITE);
    grid[7][1] = std::make_unique<Knight>(Color::WHITE);
    grid[7][2] = std::make_unique<Bishop>(Color::WHITE);
    grid[7][3] = std::make_unique<Queen>(Color::WHITE);
    grid[7][4] = std::make_unique<King>(Color::WHITE);
    grid[7][5] = std::make_unique<Bishop>(Color::WHITE);
    grid[7][6] = std::make_unique<Knight>(Color::WHITE);
    grid[7][7] = std::make_unique<Rook>(Color::WHITE);
}

bool GameLogic::isPathClear(int r1, int c1, int r2, int c2) const {
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

bool GameLogic::isSquareAttacked(int r, int c, Color attackerColor) const {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            const Piece* p = grid[i][j].get();
            if (p && p->color == attackerColor) {
                // Check if this enemy piece could move to (r,c)
                if (p->isPseudoLegal(i, j, r, c, *this)) {
                    // For sliding pieces, ensure the path is clear
                    if (p->type == PieceType::ROOK || p->type == PieceType::BISHOP || p->type == PieceType::QUEEN) {
                        if (isPathClear(i, j, r, c)) return true;
                    } else {
                        // Knights, Kings, Pawns don't need path checks
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void GameLogic::findKing(Color c, int& kr, int& kc) const {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            const Piece* p = grid[i][j].get();
            if (p && p->type == PieceType::KING && p->color == c) {
                kr = i;
                kc = j;
                return;
            }
        }
    }
}

bool GameLogic::isInCheck(Color c) const {
    int kr, kc;
    findKing(c, kr, kc);
    Color enemy = (c == Color::WHITE) ? Color::BLACK : Color::WHITE;
    return isSquareAttacked(kr, kc, enemy);
}

bool GameLogic::tryMove(const Move& m) {
    Piece* srcP = grid[m.r1][m.c1].get();
    if (!srcP) return false;
    
    Piece* destP = grid[m.r2][m.c2].get();

    // Handle En Passant simulation
    std::unique_ptr<Piece> enPassantVictim = nullptr;
    int epR = m.r1, epC = m.c2;

    if (m.isEnPassant) {
        enPassantVictim = std::move(grid[epR][epC]);
    }

    // Apply move temporarily
    std::unique_ptr<Piece> temp = std::move(grid[m.r2][m.c2]);
    grid[m.r2][m.c2] = std::move(grid[m.r1][m.c1]);

    // Check if King is safe
    bool kingSafe = !isInCheck(srcP->color);

    // Revert move
    grid[m.r1][m.c1] = std::move(grid[m.r2][m.c2]);
    grid[m.r2][m.c2] = std::move(temp);

    if (m.isEnPassant) {
        grid[epR][epC] = std::move(enPassantVictim);
    }

    return kingSafe;
}

void GameLogic::makeMove(Move m) {
    // Validate bounds
    if (m.r1 < 0 || m.r1 >= 8 || m.c1 < 0 || m.c1 >= 8 ||
        m.r2 < 0 || m.r2 >= 8 || m.c2 < 0 || m.c2 >= 8) {
        std::cout << "ERROR: Move out of bounds!\n";
        return;
    }
    
    Piece* p = grid[m.r1][m.c1].get();
    if (!p) return;

    // Track if this is a pawn move and if there's a capture
    bool isPawnMove = (p->type == PieceType::PAWN);
    bool isCapture = (grid[m.r2][m.c2] != nullptr) || m.isEnPassant;
    
    // Store moving piece type BEFORE making the move (needed for game recording)
    PieceType movingPieceType = p->type;

    // Handle En Passant capture
    if (m.isEnPassant) {
        int epRow = m.r1;
        int epCol = m.c2;
        
        // Validate en passant victim coordinates
        if (epRow < 0 || epRow >= 8 || epCol < 0 || epCol >= 8) {
            std::cout << "ERROR: En Passant victim out of bounds!\n";
            return;
        }
        
        grid[epRow][epCol] = nullptr;
        std::cout << "--- En Passant Capture! ---\n";
    }

    // Handle Castling (move the Rook)
    if (m.isCastling) {
        int row = m.r1;
        bool kingSide = (m.c2 > m.c1);
        int rookSrcCol = kingSide ? 7 : 0;
        int rookDestCol = kingSide ? 5 : 3;

        Piece* rook = grid[row][rookSrcCol].get();
        grid[row][rookDestCol] = std::move(grid[row][rookSrcCol]);
        if (rook) rook->hasMoved = true;
        std::cout << "--- Castling ---\n";
    }

    // Standard capture
    if (grid[m.r2][m.c2] != nullptr) {
        grid[m.r2][m.c2] = nullptr;
    }

    // Move the piece
    grid[m.r2][m.c2] = std::move(grid[m.r1][m.c1]);
    
    // Re-get pointer after move (since we moved the unique_ptr)
    p = grid[m.r2][m.c2].get();
    if (!p) return;  // Safety check

    // Flag for En Passant in next turn
    if (p->type == PieceType::PAWN && std::abs(m.r2 - m.r1) == 2) {
        lastMoveWasDoublePawnPush = true;
    } else {
        lastMoveWasDoublePawnPush = false;
    }

    // Pawn promotion
    if (p->type == PieceType::PAWN && (m.r2 == 0 || m.r2 == 7)) {
        Color pieceColor = p->color;
        std::cout << "PROMOTION! Pawn promoted to " << static_cast<int>(m.promotionPiece) << "\n";
        
        switch (m.promotionPiece) {
            case PieceType::KNIGHT:
                grid[m.r2][m.c2] = std::make_unique<Knight>(pieceColor);
                std::cout << "Promoted to Knight\n";
                break;
            case PieceType::BISHOP:
                grid[m.r2][m.c2] = std::make_unique<Bishop>(pieceColor);
                std::cout << "Promoted to Bishop\n";
                break;
            case PieceType::ROOK:
                grid[m.r2][m.c2] = std::make_unique<Rook>(pieceColor);
                std::cout << "Promoted to Rook\n";
                break;
            case PieceType::QUEEN:
            default:
                grid[m.r2][m.c2] = std::make_unique<Queen>(pieceColor);
                std::cout << "Promoted to Queen\n";
                break;
        }
        p = grid[m.r2][m.c2].get();
    }

    p->hasMoved = true;
    lastMove = m;
    turn = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // Trigger sound callback if set
    if (soundCallback) {
        soundCallback(isPawnMove, isCapture);
    }

    // Update game state first to check for check/checkmate
    updateGameState();
    
    // Record move in game recorder if set
    if (gameRecorder) {
        bool isCheck = isInCheck(turn);
        bool isCheckmate = checkmate;
        gameRecorder->recordMove(m, movingPieceType, isCheckmate, isCheck, isCapture);
    }
}

bool GameLogic::hasLegalMoves(Color c) {
    for (int r1 = 0; r1 < 8; r1++) {
        for (int c1 = 0; c1 < 8; c1++) {
            Piece* p = grid[r1][c1].get();
            if (p && p->color == c) {
                for (int r2 = 0; r2 < 8; r2++) {
                    for (int c2 = 0; c2 < 8; c2++) {
                        // Check geometry
                        if (p->isPseudoLegal(r1, c1, r2, c2, *this)) {
                            Move m = {r1, c1, r2, c2};

                            // Tag En Passant
                            if (p->type == PieceType::PAWN && std::abs(c1 - c2) == 1 && grid[r2][c2] == nullptr) {
                                m.isEnPassant = true;
                            }

                            // Check obstructions for sliders
                            if (p->type == PieceType::ROOK || p->type == PieceType::BISHOP || p->type == PieceType::QUEEN) {
                                if (!isPathClear(r1, c1, r2, c2)) continue;
                            }

                            // Simulate to check for Check
                            if (tryMove(m)) return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void GameLogic::updateGameState() {
    if (isInCheck(turn)) {
        if (!hasLegalMoves(turn)) {
            checkmate = true;
            gameOver = true;
            winner = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
            std::cout << "!!! CHECKMATE !!! Winner: " << (winner == Color::WHITE ? "WHITE" : "BLACK") << "\n";
        }
    } else {
        if (!hasLegalMoves(turn)) {
            stalemate = true;
            gameOver = true;
            std::cout << "!!! STALEMATE (DRAW) !!!\n";
        }
    }
}

void GameLogic::display() const {
    std::cout << "\n    a b c d e f g h\n";
    std::cout << "  +-----------------+\n";
    for (int i = 0; i < 8; i++) {
        std::cout << 8 - i << " | ";
        for (int j = 0; j < 8; j++) {
            if (grid[i][j] == nullptr) {
                std::cout << ".";
            } else {
                std::cout << grid[i][j]->getSymbol();
            }
            std::cout << " ";
        }
        std::cout << "| " << 8 - i << "\n";
    }
    std::cout << "  +-----------------+\n";
    std::cout << "    a b c d e f g h\n\n";

    std::cout << "Turn: " << (turn == Color::WHITE ? "WHITE" : "BLACK") << "\n";
}
