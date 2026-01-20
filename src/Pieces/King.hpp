#pragma once

#include "Piece.hpp"

class King : public Piece {
public:
    King(Color c) : Piece(c, PieceType::KING) {}

    char getSymbol() const override {
        return (color == Color::WHITE) ? 'K' : 'k';
    }

    std::string getCode() const override {
        return (color == Color::WHITE) ? "wK" : "bK";
    }

    bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const override;

    std::unique_ptr<Piece> clone() const override {
        return std::make_unique<King>(*this);
    }
};
