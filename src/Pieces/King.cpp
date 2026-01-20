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
    // Conditions: King moves 2 squares sideways, never moved before
    if (dr == 0 && dc == 2 && !hasMoved) {
        // Cannot castle while in check
        if (game.isInCheck(color)) return false;

        int rookCol = (c2 > c1) ? 7 : 0; // Rook at H (7) or A (0)
        const Piece* rook = game.getPiece(r1, rookCol);

        // Rook must exist, be a Rook, and not have moved
        if (rook == nullptr || rook->type != PieceType::ROOK || rook->hasMoved) return false;

        // Path between King and Rook must be empty
        if (!game.isPathClear(r1, c1, r1, rookCol)) return false;

        // The square the King crosses cannot be under attack
        int direction = (c2 > c1) ? 1 : -1;
        Color enemy = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
        if (game.isSquareAttacked(r1, c1 + direction, enemy)) return false;

        return true;
    }

    return false;
}
