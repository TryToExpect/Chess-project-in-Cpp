#include "Board.hpp"
#include "PieceManager.hpp"
#include <array>
#include <algorithm>
#include <string>
#include <cmath>
#include <iostream>
#include <cstdint>

Board::Board(float tileSize, const sf::Vector2f& origin)
    : m_tileSize(tileSize), m_origin(origin), m_currentStyle("maestro")
{
    // populate a small set of pleasing contrasting palettes (light,dark)
    m_palettes = {
        { {240,217,181}, {181,136,99} },   // classic / maestro
        { {238,238,210}, {118,150,86} },   // green classic
        { {245,245,245}, {60,60,60} },     // light/charcoal
        { {200,230,255}, {40,70,110} },    // pale blue / dark blue
        { {255,250,240}, {100,70,50} },    // warm beige / brown

        { {230,230,255}, {90,90,130} },    // lavender / deep indigo
        { {255,240,240}, {160,60,60} },    // very light rose / burgundy
        { {235,255,235}, {90,130,90} },    // mint / forest green
        { {255,255,220}, {140,120,60} },   // light sand / khaki brown
        { {225,240,245}, {70,100,120} },   // cold grey-blue / steel blue
        { {250,250,255}, {110,110,160} },  // ice white / slate violet
        { {245,240,230}, {120,90,70} }     // oatmeal / coffee brown
    };


    m_currentPaletteIndex = 0;
    // apply first palette to sf::Color members
    const auto &p = m_palettes[m_currentPaletteIndex];
    m_light = sf::Color(static_cast<uint8_t>(p.first[0]), static_cast<uint8_t>(p.first[1]), static_cast<uint8_t>(p.first[2]));
    m_dark  = sf::Color(static_cast<uint8_t>(p.second[0]), static_cast<uint8_t>(p.second[1]), static_cast<uint8_t>(p.second[2]));
}

// convert RGB triplet (0-255 ints) to sf::Color with clamping
static sf::Color rgbToColor(const Board::RGB &c) {
    auto clamp = [](int v){ return static_cast<uint8_t>(std::max(0, std::min(255, v))); };
    return sf::Color(clamp(c[0]), clamp(c[1]), clamp(c[2]));
}

// compute a simple luminance (0-255 range)
static float luminance(const Board::RGB &c) {
    return 0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2];
}

// If two RGB triplets are not contrasting enough, this returns a
// contrasting dark color (inverts and clamps).
static Board::RGB ensureContrasting(const Board::RGB &light, const Board::RGB &dark) {
    float l1 = luminance(light);
    float l2 = luminance(dark);
    if (std::fabs(l1 - l2) >= 100.f) return dark; // good enough
    // otherwise generate a contrasting dark color by inverting light
    Board::RGB alt = { 255 - light[0], 255 - light[1], 255 - light[2] };
    // if alt is still too light, force a near-black
    if (luminance(alt) > 128.f) alt = {30,30,30};
    return alt;
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

void Board::setColorsRGB(const RGB& light, const RGB& dark) {
    Board::RGB safeDark = ensureContrasting(light, dark);
    m_light = rgbToColor(light);
    m_dark = rgbToColor(safeDark);
    // also add/replace at current palette index so cycling preserves it
    if (m_currentPaletteIndex >= 0 && m_currentPaletteIndex < static_cast<int>(m_palettes.size())) {
        m_palettes[m_currentPaletteIndex] = {light, safeDark};
    }
}

std::pair<Board::RGB, Board::RGB> Board::getColorsRGB() const {
    if (m_currentPaletteIndex >= 0 && m_currentPaletteIndex < static_cast<int>(m_palettes.size())) {
        return m_palettes[m_currentPaletteIndex];
    }
    Board::RGB l = {m_light.r, m_light.g, m_light.b};
    Board::RGB d = {m_dark.r, m_dark.g, m_dark.b};
    return {l,d};
}

void Board::cyclePalette(int delta) {
    if (m_palettes.empty()) return;
    int n = static_cast<int>(m_palettes.size());
    m_currentPaletteIndex = (m_currentPaletteIndex + delta) % n;
    if (m_currentPaletteIndex < 0) m_currentPaletteIndex += n;
    const auto &p = m_palettes[m_currentPaletteIndex];
    // ensure contrast before applying
    Board::RGB safeDark = ensureContrasting(p.first, p.second);
    m_light = rgbToColor(p.first);
    m_dark = rgbToColor(safeDark);
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

void Board::setStyle(const std::string& styleName) {
    m_currentStyle = styleName;
    // Reload pieces with new style (main.cpp will handle PieceManager creation)
}
