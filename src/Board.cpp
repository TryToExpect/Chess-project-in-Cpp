#include "Board.hpp"
#include "PieceManager.hpp"
#include <array>
#include <algorithm>
#include <string>

Board::Board(float tileSize, const sf::Vector2f& origin)
    : m_tileSize(tileSize), m_origin(origin),
      m_light(240, 217, 181), // typical light square
      m_dark(181, 136, 99)    // typical dark square
{
}

void Board::setPosition(const sf::Vector2f& pos) {
    m_origin = pos;
}

sf::Vector2f Board::getSize() const {
    return {m_tileSize * 8.f, m_tileSize * 8.f};
}

void Board::setTileSize(float tileSize) {
    if (tileSize > 0.f) m_tileSize = tileSize;
}

float Board::getTileSize() const {
    return m_tileSize;
}

void Board::setPieceManager(const PieceManager* pm) {
    m_pieceManager = pm;
}

void Board::setInitialPosition() {
    // Clear
    for (auto &r : m_pieces) for (auto &c : r) c.clear();

    // Black pieces on row 0 and pawns on row 1
    const std::array<std::string,8> blackBack = {"bR","bN","bB","bQ","bK","bB","bN","bR"};
    for (int col = 0; col < 8; ++col) m_pieces[0][col] = blackBack[col];
    for (int col = 0; col < 8; ++col) m_pieces[1][col] = "bP";

    // White pawns row 6 and back row 7
    for (int col = 0; col < 8; ++col) m_pieces[6][col] = "wP";
    const std::array<std::string,8> whiteBack = {"wR","wN","wB","wQ","wK","wB","wN","wR"};
    for (int col = 0; col < 8; ++col) m_pieces[7][col] = whiteBack[col];
}

void Board::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    sf::RectangleShape square({m_tileSize, m_tileSize});

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            bool isDark = ((row + col) % 2) != 0;
            square.setFillColor(isDark ? m_dark : m_light);
            square.setPosition(sf::Vector2f(m_origin.x + col * m_tileSize, m_origin.y + row * m_tileSize));
            target.draw(square, states);
        }
    }

    // Draw pieces if any
    if (m_pieceManager) {
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                const std::string &code = m_pieces[row][col];
                if (code.empty()) continue;

                const sf::Texture* tex = m_pieceManager->getTexture(code);
                if (!tex) continue;

                sf::Sprite spr(*tex);

                // Scale sprite to fit inside tile while preserving aspect ratio
                sf::Vector2u ts = tex->getSize();
                if (ts.x == 0 || ts.y == 0) continue;
                float sx = m_tileSize / static_cast<float>(ts.x);
                float sy = m_tileSize / static_cast<float>(ts.y);
                float s = std::min(sx, sy) * 0.9f; // padding 10%
                spr.setScale({s, s});

                // position sprite centered in tile
                float spriteW = ts.x * s;
                float spriteH = ts.y * s;
                float px = m_origin.x + col * m_tileSize + (m_tileSize - spriteW) / 2.f;
                float py = m_origin.y + row * m_tileSize + (m_tileSize - spriteH) / 2.f;
                spr.setPosition({px, py});

                target.draw(spr, states);
            }
        }
    }
}
