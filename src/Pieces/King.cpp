#include "King.hpp"
#include "../GameLogic.hpp"
#include <cmath>

bool King::isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const {
    int dr = std::abs(r1 - r2);
    int dc = std::abs(c1 - c2);

    // Standard move: 1 square any direction
    if (dr <= 1 && dc <= 1) {
        const Piece* target = game.getPiece(r2, c2);
        if (target != nullptr && target->color == color) return false;
        return true;
    }

    // CASTLING Logic
    // In standard chess: King moves 2 squares sideways, never moved before
    // In Chess960: King can move any number of squares towards a Rook to castle
    
    // Check if we're attempting castling (Rook exists on destination square)
    if (dr == 0 && !hasMoved) {
        const Piece* targetSquare = game.getPiece(r2, c2);
        if (targetSquare != nullptr && targetSquare->type == PieceType::ROOK && 
            targetSquare->color == color && !targetSquare->hasMoved) {
            
            // Cannot castle while in check
            if (game.isInCheck(color)) return false;

            int rookCol = c2;
            int direction = (rookCol > c1) ? 1 : -1;

            // Path between King and Rook must be empty (excluding the rook)
            int currentCol = c1 + direction;
            while (currentCol != rookCol) {
                if (game.getPiece(r1, currentCol) != nullptr) return false;
                currentCol += direction;
            }

            // All squares the King crosses cannot be under attack
            Color enemy = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
            currentCol = c1 + direction;
            while (currentCol != rookCol + direction) {
                if (game.isSquareAttacked(r1, currentCol, enemy)) return false;
                currentCol += direction;
            }

            return true;
        }
    }

    return false;
}
