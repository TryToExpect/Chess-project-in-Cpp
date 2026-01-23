#include "bot.hpp"

#include <vector>
#include <random>
#include <cmath>

static int pieceValue(PieceType t)
{
    switch (t) {
        case PieceType::PAWN:   return 100;
        case PieceType::KNIGHT: return 300;
        case PieceType::BISHOP: return 300;
        case PieceType::ROOK:   return 500;
        case PieceType::QUEEN:  return 900;
        case PieceType::KING:   return 100000;
        default:                return 0;
    }
}

Bot::Bot(Color botColor) : color_(botColor) {}

std::optional<Move> Bot::pickMove(const GameLogic& game) const
{
    if (game.getTurn() != color_) return std::nullopt;

    GameLogic& g = const_cast<GameLogic&>(game);
    const Color enemy = (color_ == Color::WHITE) ? Color::BLACK : Color::WHITE;

    std::vector<Move> bestGoodCaptures, bestBadCaptures, safeQuietMoves, riskyQuietMoves, bestThreatEscapes;
    int bestGoodCaptureScore = -1;
    int bestBadCaptureScore  = -1;
    int bestThreatenedValue  = -1;

    for (int r1 = 0; r1 < 8; ++r1) {
        for (int c1 = 0; c1 < 8; ++c1) {

            const Piece* piece = g.getPiece(r1, c1);
            if (!piece || piece->color != color_) continue;

            const int attackerVal = pieceValue(piece->type);

            const bool threatenedNonPawn =
                piece->type != PieceType::PAWN &&
                g.isSquareAttacked(r1, c1, enemy);

            for (int r2 = 0; r2 < 8; ++r2) {
                for (int c2 = 0; c2 < 8; ++c2) {
                    if (r1 == r2 && c1 == c2) continue;
                    if (!piece->isPseudoLegal(r1, c1, r2, c2, g)) continue;

                    Move m{r1, c1, r2, c2};

                    const Piece* target = g.getPiece(r2, c2);
                    bool isCapture = (target && target->color == enemy);
                    int capturedVal = isCapture ? pieceValue(target->type) : 0;

                    // en passant
                    if (piece->type == PieceType::PAWN &&
                        std::abs(c1 - c2) == 1 &&
                        target == nullptr) {
                        m.isEnPassant = true;
                        isCapture = true;
                        capturedVal = pieceValue(PieceType::PAWN);
                    }

                    // castling
                    if (piece->type == PieceType::KING && std::abs(c1 - c2) == 2) {
                        m.isCastling = true;
                    }

                    // slidery
                    if ((piece->type == PieceType::ROOK ||
                         piece->type == PieceType::BISHOP ||
                         piece->type == PieceType::QUEEN) &&
                        !g.isPathClear(r1, c1, r2, c2)) {
                        continue;
                    }

                    if (!g.tryMove(m)) continue;

                    // ucieczka zagrożonej figury (nie pion)
                    if (threatenedNonPawn && !g.isSquareAttacked(r2, c2, enemy)) {
                        int threatenedVal = pieceValue(piece->type);
                        if (threatenedVal > bestThreatenedValue) {
                            bestThreatenedValue = threatenedVal;
                            bestThreatEscapes.clear();
                            bestThreatEscapes.push_back(m);
                        } else if (threatenedVal == bestThreatenedValue) {
                            bestThreatEscapes.push_back(m);
                        }
                    }

                    // bicie
                    if (isCapture) {
                        bool badTrade = false;
                        if (capturedVal < attackerVal && g.isSquareAttacked(r2, c2, enemy)) {
                            badTrade = true;
                        }

                        if (!badTrade) {
                            if (capturedVal > bestGoodCaptureScore) {
                                bestGoodCaptureScore = capturedVal;
                                bestGoodCaptures.clear();
                                bestGoodCaptures.push_back(m);
                            } else if (capturedVal == bestGoodCaptureScore) {
                                bestGoodCaptures.push_back(m);
                            }
                        } else {
                            if (capturedVal > bestBadCaptureScore) {
                                bestBadCaptureScore = capturedVal;
                                bestBadCaptures.clear();
                                bestBadCaptures.push_back(m);
                            } else if (capturedVal == bestBadCaptureScore) {
                                bestBadCaptures.push_back(m);
                            }
                        }
                        continue;
                    }

                    // ruchy ciche
                    if (!g.isSquareAttacked(r2, c2, enemy)) safeQuietMoves.push_back(m);
                    else riskyQuietMoves.push_back(m);
                }
            }
        }
    }

    static thread_local std::mt19937 rng{std::random_device{}()};

    // 1) ucieczka zagrożonej figury (jeśli bicie nie zbija >= jej wartości)
    if (!bestThreatEscapes.empty()) {
        bool captureBetter = !bestGoodCaptures.empty() && bestGoodCaptureScore >= bestThreatenedValue;
        if (!captureBetter) {
            std::uniform_int_distribution<size_t> d(0, bestThreatEscapes.size() - 1);
            return bestThreatEscapes[d(rng)];
        }
    }

    // 2) najlepsze bicie
    if (!bestGoodCaptures.empty()) {
        std::uniform_int_distribution<size_t> d(0, bestGoodCaptures.size() - 1);
        return bestGoodCaptures[d(rng)];
    }

    // 3) ruchy ciche
    if (!safeQuietMoves.empty()) {
        std::uniform_int_distribution<size_t> d(0, safeQuietMoves.size() - 1);
        return safeQuietMoves[d(rng)];
    }
    if (!riskyQuietMoves.empty()) {
        std::uniform_int_distribution<size_t> d(0, riskyQuietMoves.size() - 1);
        return riskyQuietMoves[d(rng)];
    }

    // 4) ostateczność: złe bicia
    if (!bestBadCaptures.empty()) {
        std::uniform_int_distribution<size_t> d(0, bestBadCaptures.size() - 1);
        return bestBadCaptures[d(rng)];
    }

    return std::nullopt;
}
