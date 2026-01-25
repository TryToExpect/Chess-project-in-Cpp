#include "GameRecorder.hpp"
#include "Pieces/Piece.hpp"
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

GameRecorder::GameRecorder() 
    : gameResult(GameResult::UNKNOWN), endReason("unknown") {
    variant = "";
    // Create recent_games directory if it doesn't exist
    // The directory is created relative to the build directory's parent
    fs::path projectRoot = fs::path(__FILE__).parent_path().parent_path();
    fs::path gamesDir = projectRoot / "recent_games";
    
    if (!fs::exists(gamesDir)) {
        try {
            fs::create_directories(gamesDir);
            std::cout << "Created recent_games directory: " << gamesDir << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
        }
    }
}

std::string GameRecorder::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
    return ss.str();
}

std::string GameRecorder::resultToString(GameResult result) const {
    switch (result) {
        case GameResult::WHITE_WIN_CHECKMATE:
            return "white_win";
        case GameResult::BLACK_WIN_CHECKMATE:
            return "black_win";
        case GameResult::STALEMATE:
            return "stalemate";
        case GameResult::WHITE_TIMEOUT:
            return "white_timeout";
        case GameResult::BLACK_TIMEOUT:
            return "black_timeout";
        case GameResult::DRAW_BY_AGREEMENT:
            return "draw_agreement";
        default:
            return "unknown";
    }
}

std::string GameRecorder::getResultSymbol(GameResult result) const {
    switch (result) {
        case GameResult::WHITE_WIN_CHECKMATE:
            return "1-0";
        case GameResult::BLACK_WIN_CHECKMATE:
            return "0-1";
        case GameResult::STALEMATE:
        case GameResult::DRAW_BY_AGREEMENT:
            return "1/2-1/2";
        case GameResult::WHITE_TIMEOUT:
            return "0-1[timeout]";
        case GameResult::BLACK_TIMEOUT:
            return "1-0[timeout]";
        default:
            return "*";
    }
}

std::string GameRecorder::generateFilename(GameResult result, const std::string& reason) {
    // Format: YYYYMMDD_HHMMSS_[wynik]_[powód]
    // Example: 20260122_143025_white_win_checkmate.txt
    // Example: 20260122_143530_black_timeout.txt
    
    std::string dateTime = getCurrentDateTime();
    std::string resultStr = resultToString(result);
    std::string reasonStr = reason;
    
    // Replace spaces with underscores
    for (char& c : reasonStr) {
        if (c == ' ') c = '_';
    }
    
    return dateTime + "_" + resultStr + "_" + reasonStr + ".txt";
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

void GameRecorder::endGame(GameResult result, const std::string& reason) {
    gameResult = result;
    endReason = reason;
    filename = generateFilename(result, reason);
}

void GameRecorder::saveToFile() {
    // Path to recent_games directory (at project root level, same as src, build)
    fs::path projectRoot = fs::path(__FILE__).parent_path().parent_path();
    fs::path gamesDir = projectRoot / "recent_games";
    fs::path targetDir = gamesDir;
    if (!variant.empty()) {
        // sanitize variant: avoid absolute paths
        fs::path var = fs::path(variant);
        if (var.has_root_directory()) {
            // ignore root, use only filename part
            var = var.filename();
        }
        targetDir = gamesDir / var;
    }

    // Ensure target directory exists
    if (!fs::exists(targetDir)) {
        try {
            fs::create_directories(targetDir);
        } catch (const std::exception& e) {
            std::cerr << "Error creating directory: " << e.what() << "\n";
            throw;
        }
    }

    fs::path filepath = targetDir / filename;

    std::ofstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath.string() << " for writing.\n";
        throw std::runtime_error("Cannot open file: " + filepath.string());
    }

    // Write moves in format: 1. e4 e5
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

    file << "\n" << getResultSymbol(gameResult) << "\n";
    file << "Reason: " << endReason << "\n";

    file.close();

    std::cout << "Game saved to: " << filepath.string() << std::endl;
}

void GameRecorder::clear() {
    moves.clear();
    gameResult = GameResult::UNKNOWN;
    endReason = "unknown";
    filename = "";
}