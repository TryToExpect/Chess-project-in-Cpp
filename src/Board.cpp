#include "Board.hpp"

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
}
