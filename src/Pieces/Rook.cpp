#include "Rook.hpp"
#include "../GameLogic.hpp"

bool Rook::isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const {
    // Rook moves in straight lines: either same row or same column
    if (r1 != r2 && c1 != c2) return false;

    // Cannot capture own piece
    const Piece* target = game.getPiece(r2, c2);
    if (target != nullptr && target->color == color) return false;

    // Note: Path obstruction is checked in GameLogic::isPathClear
    return true;
}
