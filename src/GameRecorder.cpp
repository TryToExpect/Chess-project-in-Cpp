#include "GameRecorder.hpp"
#include "Pieces/Piece.hpp"
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>

GameRecorder::GameRecorder() {
    filename = getCurrentDateTime() + ".txt";
}

std::string GameRecorder::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

std::string GameRecorder::coordinatesToAlgebraic(int row, int col) const {
    std::string result;
    result += char('a' + col);  // Column: a-h
    result += char('8' - row);  // Row: 8-1
    return result;
}

char GameRecorder::getPieceSymbol(PieceType type) const {
    switch (type) {
        case PieceType::KING:   return 'K';
        case PieceType::QUEEN:  return 'Q';
        case PieceType::ROOK:   return 'R';
        case PieceType::BISHOP: return 'B';
        case PieceType::KNIGHT: return 'N';
        case PieceType::PAWN:   return '\0'; // Empty for pawn
        case PieceType::EMPTY:  return '\0';
        default:                return '\0';
    }
}

void GameRecorder::recordMove(const Move& move, PieceType movingPiece,
                              bool isCheckmate, bool isCheck, bool isCapture) {
    RecordedMove rm;
    rm.move = move;
    rm.isCheckmate = isCheckmate;
    rm.isCheck = isCheck;
    rm.isCapture = isCapture;
    rm.movingPiece = movingPiece;
    
    moves.push_back(rm);
}

void GameRecorder::saveToFile(const std::string& result) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    // Write moves in standard algebraic notation
    int moveNumber = 1;
    for (size_t i = 0; i < moves.size(); i++) {
        if (i % 2 == 0) {
            // White move - start with move number
            file << moveNumber << ". ";
        }
        
        const RecordedMove& rm = moves[i];
        const Move& move = rm.move;
        
        // Handle castling
        if (move.isCastling) {
            if (move.c2 > move.c1) {
                file << "O-O";
            } else {
                file << "O-O-O";
            }
        } else {
            // Add piece symbol (except for pawns)
            if (rm.movingPiece != PieceType::PAWN && rm.movingPiece != PieceType::EMPTY) {
                file << getPieceSymbol(rm.movingPiece);
            }
            
            // Add capture symbol
            if (rm.isCapture) {
                // For pawns, add source file
                if (rm.movingPiece == PieceType::PAWN) {
                    file << char('a' + move.c1);
                }
                file << "x";
            }
            
            // Add destination square
            file << coordinatesToAlgebraic(move.r2, move.c2);
            
            // Add promotion if applicable
            if (move.isPromotion) {
                file << "=";
                switch (move.promotionPiece) {
                    case PieceType::QUEEN:  file << "Q"; break;
                    case PieceType::ROOK:   file << "R"; break;
                    case PieceType::BISHOP: file << "B"; break;
                    case PieceType::KNIGHT: file << "N"; break;
                    default: file << "Q"; break;
                }
            }
        }
        
        // Check and checkmate indicators
        if (rm.isCheckmate) {
            file << "#";
        } else if (rm.isCheck) {
            file << "+";
        }
        
        if (i % 2 == 1) {
            // Black move - end line
            file << "\n";
            moveNumber++;
        } else {
            // White move - add space for black's move
            file << " ";
        }
    }
    
    // Add result on new line if last move was white's
    if (moves.size() % 2 == 1) {
        file << "\n";
    }
    
    file << result << "\n";
    
    file.close();
    
    std::cout << "Game saved to: " << filename << "\n";
}

void GameRecorder::clear() {
    moves.clear();
}
