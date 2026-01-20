#pragma once

#include "Piece.hpp"

class Pawn : public Piece {
public:
    Pawn(Color c) : Piece(c, PieceType::PAWN) {}

    char getSymbol() const override {
        return (color == Color::WHITE) ? 'P' : 'p';
    }

    std::string getCode() const override {
        return (color == Color::WHITE) ? "wP" : "bP";
    }

    bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const override;

    std::unique_ptr<Piece> clone() const override {
        return std::make_unique<Pawn>(*this);
    }
};
