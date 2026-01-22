#pragma once

#include <array>
#include <memory>
#include <functional>
#include <random>
#include "Pieces/Piece.hpp"

// Forward declarations
class GameRecorder;
enum class GameResult;

struct Move {
    int r1, c1, r2, c2;         // Source and Destination coordinates
    bool isEnPassant = false;   // Special move flag: En Passant
    bool isCastling = false;    // Special move flag: Castling
    bool isPromotion = false;   // Special move flag: Pawn Promotion
    PieceType promotionPiece = PieceType::QUEEN; // Default promotion is Queen
};

class GameLogic {
public:
    using Grid = std::array<std::array<std::unique_ptr<Piece>, 8>, 8>;

    GameLogic();
    ~GameLogic() = default;

    // Setup standard chess starting position
    void setup();

    // Setup Chess960 (Fischer random chess) starting position
    void setupFischer();

    // Get piece at position
    Piece* getPiece(int r, int c) { return grid[r][c].get(); }
    const Piece* getPiece(int r, int c) const { return grid[r][c].get(); }

    // Check if path is clear (for sliding pieces: Rook, Bishop, Queen)
    bool isPathClear(int r1, int c1, int r2, int c2) const;

    // Check if specific square is under attack by opponent
    bool isSquareAttacked(int r, int c, Color attackerColor) const;

    // Check if current player's King is in Check
    bool isInCheck(Color c) const;

    // Find King of given color
    void findKing(Color c, int& kr, int& kc) const;

    // Simulate move to check if it leaves King in check
    bool tryMove(const Move& m);

    // Execute move permanently
    void makeMove(Move m);

    // Check if player has any legal moves (for Checkmate/Stalemate)
    bool hasLegalMoves(Color c);

    // Get current turn
    Color getTurn() const { return turn; }

    // Get last move and en passant status
    const Move& getLastMove() const { return lastMove; }
    bool isLastMoveDoublePawnPush() const { return lastMoveWasDoublePawnPush; }

    // Display board (for debugging)
    void display() const;

    // Check game state
    bool isGameOver() const { return gameOver; }
    bool isCheckmate() const { return checkmate; }
    bool isStalemate() const { return stalemate; }
    Color getWinner() const { return winner; }

    // Update game state (call after every move to check for mate/stalemate)
    void updateGameState();

    // Callback for sound events
    using SoundCallback = std::function<void(bool isPawnMove, bool isCapture)>;
    void setSoundCallback(SoundCallback callback) { soundCallback = callback; }

    // Game recorder
    void setGameRecorder(GameRecorder* recorder) { gameRecorder = recorder; }
    
    // End game with result and reason
    void endGameWithResult(GameResult result, const std::string& reason);

    // Check if this is a Chess960 game
    bool isChess960Game() const { return isChess960; }

private:
    Grid grid;
    Color turn;
    Move lastMove;
    bool lastMoveWasDoublePawnPush;

    // Game state
    bool gameOver = false;
    bool checkmate = false;
    bool stalemate = false;
    Color winner = Color::NONE;

    // Chess960 flag
    bool isChess960 = false;

    // Sound callback
    SoundCallback soundCallback;

    // Game recorder pointer (not owned)
    GameRecorder* gameRecorder = nullptr;
};
