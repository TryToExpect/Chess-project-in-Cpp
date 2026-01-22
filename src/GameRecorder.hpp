#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "GameLogic.hpp"

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
    
    // Helper methods
    std::string getCurrentDateTime();
    
    // Convert board coordinates to algebraic notation (e.g., e4)
    std::string coordinatesToAlgebraic(int row, int col) const;
    
    // Get piece symbol (K, Q, R, B, N for non-pawns)
    char getPieceSymbol(PieceType type) const;

public:
    GameRecorder();
    
    // Record a move with game state information
    void recordMove(const Move& move, PieceType movingPiece, bool isCheckmate, bool isCheck, bool isCapture);
    
    // Save all recorded moves to a .txt file with result
    void saveToFile(const std::string& result = "1/2-1/2"); // Default to draw, can be "1-0", "0-1", "1/2-1/2"
    
    // Clear all recorded moves
    void clear();
    
    // Get number of recorded moves
    size_t getMoveCount() const { return moves.size(); }
    
    // Get the filename that will be used for saving
    std::string getFilename() const { return filename; }
};

