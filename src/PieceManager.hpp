#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>

// Loads piece textures from subdirectories under assets/pieces/<style>/
// Supports common raster formats supported by your SFML build. For SVG
// files loading depends on your SFML build/plugins; if unsupported the
// texture will fail to load and the piece won't be drawn.
class PieceManager {
public:
    // Scan assets/pieces directory and return available style names
    static std::vector<std::string> listAvailableStyles(const std::string& baseDir = "assets/pieces");

    // Construct and load a style (folder name under assets/pieces)
    explicit PieceManager(const std::string& styleName, const std::string& baseDir = "assets/pieces");

    // Get pointer to texture by piece code like "wP", "bK". Returns nullptr if not found.
    const sf::Texture* getTexture(const std::string& pieceCode) const;

    // Check whether loading succeeded
    bool isLoaded() const { return m_loaded; }

private:
    bool m_loaded = false;
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::string m_style;
};
