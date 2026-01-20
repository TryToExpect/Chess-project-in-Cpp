#pragma once

#include "Piece.hpp"

class Queen : public Piece {
public:
    Queen(Color c) : Piece(c, PieceType::QUEEN) {}

    char getSymbol() const override {
        return (color == Color::WHITE) ? 'Q' : 'q';
    }

    std::string getCode() const override {
        return (color == Color::WHITE) ? "wQ" : "bQ";
    }

    bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const override;

    std::unique_ptr<Piece> clone() const override {
        return std::make_unique<Queen>(*this);
    }
};
