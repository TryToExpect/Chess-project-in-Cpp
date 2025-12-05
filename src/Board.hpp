// Simple Board class for drawing an 8x8 chessboard using SFML
#pragma once

#include <SFML/Graphics.hpp>

class Board : public sf::Drawable {
public:
    // tileSize in pixels, origin is where top-left will be placed
    Board(float tileSize = 80.f, const sf::Vector2f& origin = {0.f, 0.f});

    // set/get position of board origin
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getSize() const;

    // change tile size at runtime (keeps board square by changing tile size)
    void setTileSize(float tileSize);
    float getTileSize() const;

    // Piece rendering: attach a PieceManager to provide textures
    void setPieceManager(const class PieceManager* pm);

    // initialize pieces to standard chess starting position
    void setInitialPosition();

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    float m_tileSize;
    sf::Vector2f m_origin;
    sf::Color m_light;
    sf::Color m_dark;
    // pointer to piece textures provider (not owned)
    const class PieceManager* m_pieceManager = nullptr;
    // board piece codes, row-major [row][col], empty string -> no piece
    std::array<std::array<std::string, 8>, 8> m_pieces;
};
