#include "Pawn.hpp"
#include "../GameLogic.hpp"
#include <cmath>

bool Pawn::isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const {
    int direction = (color == Color::WHITE) ? -1 : 1; // White moves up (-1), Black moves down (+1)
    int startRow = (color == Color::WHITE) ? 6 : 1;

    // Move forward 1 square
    if (c1 == c2 && r2 == r1 + direction) {
        return game.getPiece(r2, c2) == nullptr;
    }

    // Move forward 2 squares (from starting position)
    if (c1 == c2 && r2 == r1 + 2 * direction && r1 == startRow) {
        return game.getPiece(r1 + direction, c1) == nullptr && game.getPiece(r2, c2) == nullptr;
    }

    // Diagonal capture or En Passant
    if (std::abs(c1 - c2) == 1 && r2 == r1 + direction) {
        const Piece* target = game.getPiece(r2, c2);
        if (target != nullptr && target->color != color) {
            return true; // Standard capture
        }

        // EN PASSANT CHECK: Target is empty, but opponent's pawn did double push
        if (target == nullptr && game.isLastMoveDoublePawnPush()) {
            // Check if opponent's pawn is adjacent to us
            if (game.getLastMove().r2 == r1 && game.getLastMove().c2 == c2) {
                return true;
            }
        }
    }

    return false;
}
