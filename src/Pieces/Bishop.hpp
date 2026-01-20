#pragma once

#include "Piece.hpp"

class Bishop : public Piece {
public:
    Bishop(Color c) : Piece(c, PieceType::BISHOP) {}

    char getSymbol() const override {
        return (color == Color::WHITE) ? 'B' : 'b';
    }

    std::string getCode() const override {
        return (color == Color::WHITE) ? "wB" : "bB";
    }

    bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const override;

    std::unique_ptr<Piece> clone() const override {
        return std::make_unique<Bishop>(*this);
    }
};
