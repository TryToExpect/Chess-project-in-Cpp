#pragma once

#include <string>
#include <memory>

enum class Color { WHITE, BLACK, NONE };
enum class PieceType { KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN, EMPTY };

// Forward declaration
class GameLogic;

// Abstract base class for all chess pieces
class Piece {
public:
    Color color;
    PieceType type;
    bool hasMoved; // Track if piece has moved (essential for Castling and Pawn initial move)

    Piece(Color c, PieceType t) : color(c), type(t), hasMoved(false) {}
    virtual ~Piece() = default;

    // Pure virtual function to get the character symbol of the piece
    virtual char getSymbol() const = 0;

    // Get piece code for texture loading (e.g., "wP", "bR")
    virtual std::string getCode() const = 0;

    // Checks if the move is geometrically valid for the piece type.
    // DOES NOT check for obstructions (except for Pawn capture logic) or King safety.
    virtual bool isPseudoLegal(int r1, int c1, int r2, int c2, const GameLogic& game) const = 0;

    // Create a copy of this piece
    virtual std::unique_ptr<Piece> clone() const = 0;
};
