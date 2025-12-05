#include "PieceManager.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::vector<std::string> PieceManager::listAvailableStyles(const std::string& baseDir) {
    std::vector<std::string> styles;
    try {
        if (fs::exists(baseDir) && fs::is_directory(baseDir)) {
            for (auto &p : fs::directory_iterator(baseDir)) {
                if (p.is_directory()) styles.push_back(p.path().filename().string());
            }
        }
    } catch (const std::exception &e) {
        (void)e;
    }
    return styles;
}

PieceManager::PieceManager(const std::string& styleName, const std::string& baseDir)
    : m_style(styleName) {
    m_loaded = false;
    std::string dir = baseDir + "/" + styleName;
    try {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "Piece style folder not found: " << dir << "\n";
            return;
        }

        // Expected file names: wK, wQ, wR, wB, wN, wP and bK... etc with extensions
        const std::vector<std::string> codes = {"wK","wQ","wR","wB","wN","wP","bK","bQ","bR","bB","bN","bP"};

        for (const auto &code : codes) {
            // find any file that starts with the code
            bool found = false;
            for (auto &p : fs::directory_iterator(dir)) {
                if (!p.is_regular_file()) continue;
                std::string name = p.path().filename().string();
                // case-sensitive match prefix
                if (name.rfind(code, 0) == 0) {
                    sf::Texture tex;
                    if (tex.loadFromFile(p.path().string())) {
                        m_textures.emplace(code, std::move(tex));
                        found = true;
                        break;
                    } else {
                        std::cerr << "Failed to load texture: " << p.path() << '\n';
                    }
                }
            }
            if (!found) {
                std::cerr << "Missing piece texture for code: " << code << " in style " << styleName << '\n';
            }
        }

        m_loaded = true;
    } catch (const std::exception &e) {
        std::cerr << "Error loading piece style: " << e.what() << '\n';
        m_loaded = false;
    }
}

const sf::Texture* PieceManager::getTexture(const std::string& pieceCode) const {
    auto it = m_textures.find(pieceCode);
    if (it == m_textures.end()) return nullptr;
    return &it->second;
}
