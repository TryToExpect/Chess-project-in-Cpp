#pragma once

#include "Piece.hpp"

class Knight : public Piece {
public:
    Knight(Color c) : Piece(c, PieceType::KNIGHT) {}

    char getSymbol() const override {
        return (color == Color::WHITE) ? 'N' : 'n';
    }

    std::string getCode() const override {
        return (color == Color::WHITE) ? "wN" : "bN";
    }

    bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const override;

    std::unique_ptr<Piece> clone() const override {
        return std::make_unique<Knight>(*this);
    }
};
