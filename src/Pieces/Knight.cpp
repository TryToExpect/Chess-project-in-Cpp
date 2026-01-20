#include "Knight.hpp"
#include "../GameLogic.hpp"
#include <cmath>

bool Knight::isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const {
    int dr = std::abs(r1 - r2);
    int dc = std::abs(c1 - c2);

    // Knight moves in L-shape: 2x1 or 1x2
    if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;

    // Cannot capture own piece
    const Piece* target = game.getPiece(r2, c2);
    if (target != nullptr && target->color == color) return false;

    return true;
}
