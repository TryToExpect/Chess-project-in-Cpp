// Simple Board class for drawing an 8x8 chessboard using SFML
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <utility>

class Board : public sf::Drawable {
public:
    using RGB = std::array<int,3>;
    // tileSize in pixels, origin is where top-left will be placed
    Board(float tileSize = 80.f, const sf::Vector2f& origin = {0.f, 0.f});

    // set/get position of board origin
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getSize() const;

    // Set colors using RGB triplets (values 0-255). If provided pair isn't
    // sufficiently contrasting, the dark color will be replaced by an
    // automatically chosen contrasting color.
    void setColorsRGB(const RGB& light, const RGB& dark);
    std::pair<RGB, RGB> getColorsRGB() const;
    // Cycle through internal palettes (delta = +1 or -1)
    void cyclePalette(int delta);
    int getCurrentPaletteIndex() const { return m_currentPaletteIndex; }

    // change tile size at runtime (keeps board square by changing tile size)
    void setTileSize(float tileSize);
    float getTileSize() const;

    // Piece rendering: attach a PieceManager to provide textures
    void setPieceManager(const class PieceManager* pm);

    // initialize pieces to standard chess starting position
    void setInitialPosition();

    // initialize pieces to Chess960 (Fischer random chess) position
    void setInitialFischerPosition();

    // get current piece style name
    std::string getCurrentStyle() const { return m_currentStyle; }

    // set piece style (reloads textures and pieces)
    void setStyle(const std::string& styleName);

    // Update board display from GameLogic
    void updateFromGame(const class GameLogic& game);

    // Mark/unmark squares (for right-click highlighting)
    void toggleMarkSquare(int row, int col);
    void clearMarkedSquares();
    bool isSquareMarked(int row, int col) const;

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
    // current piece style name
    std::string m_currentStyle;
    // color palettes stored as pairs of RGB triplets (light, dark)
    std::vector<std::pair<RGB, RGB>> m_palettes;
    int m_currentPaletteIndex = 0;
    // base directory for piece assets
    std::string m_assetDir = "../assets/pieces";
    // marked squares for highlighting (set of (row, col) pairs)
    std::vector<std::pair<int, int>> m_markedSquares;
};
