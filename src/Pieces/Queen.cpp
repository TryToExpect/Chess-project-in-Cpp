#include "Queen.hpp"
#include "../GameLogic.hpp"
#include <cmath>

bool Queen::isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const {
    // Queen combines Rook and Bishop logic
    bool straight = (r1 == r2 || c1 == c2);
    bool diagonal = (std::abs(r1 - r2) == std::abs(c1 - c2));

    if (!straight && !diagonal) return false;

    // Cannot capture own piece
    const Piece* target = game.getPiece(r2, c2);
    if (target != nullptr && target->color == color) return false;

    // Note: Path obstruction is checked in GameLogic::isPathClear
    return true;
}
