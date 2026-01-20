#pragma once

#include "Piece.hpp"

class Rook : public Piece {
public:
    Rook(Color c) : Piece(c, PieceType::ROOK) {}

    char getSymbol() const override {
        return (color == Color::WHITE) ? 'R' : 'r';
    }

    std::string getCode() const override {
        return (color == Color::WHITE) ? "wR" : "bR";
    }

    bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const override;

    std::unique_ptr<Piece> clone() const override {
        return std::make_unique<Rook>(*this);
    }
};
