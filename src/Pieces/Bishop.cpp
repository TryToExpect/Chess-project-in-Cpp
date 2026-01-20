#include "Bishop.hpp"
#include "../GameLogic.hpp"
#include <cmath>

bool Bishop::isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const {
    // Bishop moves diagonally: absolute change in row must equal absolute change in column
    if (std::abs(r1 - r2) != std::abs(c1 - c2)) return false;

    // Cannot capture own piece
    const Piece* target = game.getPiece(r2, c2);
    if (target != nullptr && target->color == color) return false;

    // Note: Path obstruction is checked in GameLogic::isPathClear
    return true;
}
