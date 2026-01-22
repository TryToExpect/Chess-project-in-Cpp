#include "Board.hpp"
#include "PieceManager.hpp"
#include "GameLogic.hpp"
#include "Pieces/Piece.hpp"
#include <array>
#include <algorithm>
#include <string>

Board::Board(float tileSize, const sf::Vector2f& origin)
    : m_tileSize(tileSize), m_origin(origin),
      m_light(240, 217, 181), // typical light square
      m_dark(181, 136, 99),   // typical dark square
      m_currentStyle("maestro") // default style
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
void Board::setInitialFischerPosition(){
    // Clear the board
    for (auto &r : m_pieces) for (auto &c : r) c.clear();
    
    // Place Pawns (rows 1 and 6) - same as standard chess
    for (int col = 0; col < 8; ++col) m_pieces[1][col] = "bP";
    for (int col = 0; col < 8; ++col) m_pieces[6][col] = "wP";
    
    // Note: The actual Chess960 position generation will be handled by GameLogic
    // This method just provides the UI update mechanism
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
            
            // Draw marked square overlay (semi-transparent red)
            if (isSquareMarked(row, col)) {
                sf::RectangleShape markedOverlay({m_tileSize, m_tileSize});
                markedOverlay.setFillColor(sf::Color(255, 0, 0, 80));  // Red with alpha=80
                markedOverlay.setPosition(sf::Vector2f(m_origin.x + col * m_tileSize, m_origin.y + row * m_tileSize));
                target.draw(markedOverlay, states);
            }
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

    // Draw arrows (planned moves)
    for (const auto& arrow : m_arrows) {
        drawArrow(target, states, arrow);
    }
}

void Board::drawArrow(sf::RenderTarget& target, sf::RenderStates states, const Arrow& arrow) const {
    // Calculate center points of source and destination squares
    float fromX = m_origin.x + arrow.fromCol * m_tileSize + m_tileSize / 2.f;
    float fromY = m_origin.y + arrow.fromRow * m_tileSize + m_tileSize / 2.f;
    float toX = m_origin.x + arrow.toCol * m_tileSize + m_tileSize / 2.f;
    float toY = m_origin.y + arrow.toRow * m_tileSize + m_tileSize / 2.f;

    // Direction vector
    float dx = toX - fromX;
    float dy = toY - fromY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length < 0.1f) return; // Prevent division by zero
    
    // Normalize direction
    float dirX = dx / length;
    float dirY = dy / length;
    
    // Arrow shaft width
    float shaftWidth = m_tileSize * 0.12f;
    float arrowHeadSize = m_tileSize * 0.25f;
    
    // Arrow head point (shorten line so head doesn't overlap)
    float endX = toX - dirX * arrowHeadSize * 0.7f;
    float endY = toY - dirY * arrowHeadSize * 0.7f;
    
    // Perpendicular vector for shaft width
    float perpX = -dirY;
    float perpY = dirX;
    
    // Draw arrow shaft as a quadrilateral
    sf::ConvexShape shaft(4);
    shaft.setPoint(0, sf::Vector2f(fromX - perpX * shaftWidth / 2.f, fromY - perpY * shaftWidth / 2.f));
    shaft.setPoint(1, sf::Vector2f(fromX + perpX * shaftWidth / 2.f, fromY + perpY * shaftWidth / 2.f));
    shaft.setPoint(2, sf::Vector2f(endX + perpX * shaftWidth / 2.f, endY + perpY * shaftWidth / 2.f));
    shaft.setPoint(3, sf::Vector2f(endX - perpX * shaftWidth / 2.f, endY - perpY * shaftWidth / 2.f));
    shaft.setFillColor(arrow.color);
    target.draw(shaft, states);
    
    // Draw arrow head (triangle)
    sf::ConvexShape head(3);
    float headPerpX = perpX * arrowHeadSize;
    float headPerpY = perpY * arrowHeadSize;
    head.setPoint(0, sf::Vector2f(toX, toY)); // tip
    head.setPoint(1, sf::Vector2f(endX - headPerpX * 0.5f, endY - headPerpY * 0.5f));
    head.setPoint(2, sf::Vector2f(endX + headPerpX * 0.5f, endY + headPerpY * 0.5f));
    head.setFillColor(arrow.color);
    target.draw(head, states);
}
void Board::setStyle(const std::string& styleName) {
    m_currentStyle = styleName;
    // Reload pieces with new style (main.cpp will handle PieceManager creation)
}

void Board::updateFromGame(const GameLogic& game) {
    // Clear current board
    for (auto& r : m_pieces) for (auto& c : r) c.clear();

    // Update board with pieces from GameLogic
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = game.getPiece(row, col);
            if (piece) {
                m_pieces[row][col] = piece->getCode();
            }
        }
    }
}

// Color palettes for chess boards
void Board::setColorsRGB(const RGB& light, const RGB& dark) {
    m_light = sf::Color(light[0], light[1], light[2]);
    m_dark = sf::Color(dark[0], dark[1], dark[2]);
}

std::pair<Board::RGB, Board::RGB> Board::getColorsRGB() const {
    Board::RGB light = {m_light.r, m_light.g, m_light.b};
    Board::RGB dark = {m_dark.r, m_dark.g, m_dark.b};
    return {light, dark};
}

void Board::cyclePalette(int delta) {
    // Initialize palettes if empty
    if (m_palettes.empty()) {
        m_palettes = {
            {{240, 217, 181}, {181, 136, 99}},   // Maestro
            {{240, 217, 181}, {117, 101, 70}},   // Classic
            {{230, 230, 230}, {60, 60, 60}},     // Gray
            {{255, 250, 205}, {102, 102, 102}},  // Light
            {{219, 195, 152}, {130, 98, 58}},    // Wood
        };
    }

    m_currentPaletteIndex = (m_currentPaletteIndex + delta + m_palettes.size()) % m_palettes.size();
    setColorsRGB(m_palettes[m_currentPaletteIndex].first, m_palettes[m_currentPaletteIndex].second);
}

// Mark/unmark squares for highlighting
void Board::toggleMarkSquare(int row, int col) {
    // Check if square is already marked
    auto it = std::find(m_markedSquares.begin(), m_markedSquares.end(), std::make_pair(row, col));
    if (it != m_markedSquares.end()) {
        // Square is marked, remove it
        m_markedSquares.erase(it);
    } else {
        // Square is not marked, add it
        m_markedSquares.push_back({row, col});
    }
}

void Board::clearMarkedSquares() {
    m_markedSquares.clear();
}

bool Board::isSquareMarked(int row, int col) const {
    return std::find(m_markedSquares.begin(), m_markedSquares.end(), std::make_pair(row, col)) != m_markedSquares.end();
}

// Arrow management methods
void Board::addArrow(int fromRow, int fromCol, int toRow, int toCol, const sf::Color& color) {
    // Check if arrow already exists (to avoid duplicates)
    for (const auto& arrow : m_arrows) {
        if (arrow.fromRow == fromRow && arrow.fromCol == fromCol && 
            arrow.toRow == toRow && arrow.toCol == toCol) {
            return; // Arrow already exists
        }
    }
    m_arrows.push_back(Arrow(fromRow, fromCol, toRow, toCol, color));
}

void Board::clearArrows() {
    m_arrows.clear();
}

void Board::removeArrow(int fromRow, int fromCol, int toRow, int toCol) {
    auto it = std::remove_if(m_arrows.begin(), m_arrows.end(),
        [fromRow, fromCol, toRow, toCol](const Arrow& arrow) {
            return arrow.fromRow == fromRow && arrow.fromCol == fromCol && 
                   arrow.toRow == toRow && arrow.toCol == toCol;
        });
    m_arrows.erase(it, m_arrows.end());
}
