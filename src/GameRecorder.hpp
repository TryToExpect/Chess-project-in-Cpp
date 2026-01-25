#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "GameLogic.hpp"

// Game result types
enum class GameResult {
    WHITE_WIN_CHECKMATE,
    BLACK_WIN_CHECKMATE,
    STALEMATE,
    WHITE_TIMEOUT,
    BLACK_TIMEOUT,
    DRAW_BY_AGREEMENT,
    UNKNOWN
};

// Represents recorded move data
struct RecordedMove {
    Move move;
    PieceType movingPiece;  // Type of piece that moved
    bool isCheckmate;
    bool isCheck;
    bool isCapture;
};

class GameRecorder {
private:
    std::vector<RecordedMove> moves; // All recorded moves
    std::string filename;
    GameResult gameResult;
    std::string endReason; // "checkmate", "timeout", "stalemate", etc.
    std::string variant; // game variant / subdirectory under recent_games
    
    // Helper methods
    std::string getCurrentDateTime();
    std::string generateFilename(GameResult result, const std::string& reason);
    std::string resultToString(GameResult result) const;
    std::string getResultSymbol(GameResult result) const;
    
    // Convert board coordinates to algebraic notation (e.g., e4)
    std::string coordinatesToAlgebraic(int row, int col) const;
    
    // Get piece symbol (K, Q, R, B, N for non-pawns)
    char getPieceSymbol(PieceType type) const;

public:
    GameRecorder();

    // Set/get game variant (e.g. "standard", "fischer", "diagonal", "cylinder")
    void setVariant(const std::string& v) { variant = v; }
    std::string getVariant() const { return variant; }
    
    // Record a move with game state information
    void recordMove(const Move& move, PieceType movingPiece, bool isCheckmate, bool isCheck, bool isCapture);
    
    // End game with result and reason
    void endGame(GameResult result, const std::string& reason);
    
    // Save all recorded moves to a .txt file with result
    void saveToFile();
    
    // Clear all recorded moves
    void clear();
    
    // Get number of recorded moves
    size_t getMoveCount() const { return moves.size(); }
    
    // Get the filename that will be used for saving
    std::string getFilename() const { return filename; }
};

