#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include "Board.hpp"
#include "PieceManager.hpp"
#include <memory>
#include <iostream>

int main() {
    // Fixed tile size — board won't scale on window resize
    const float tileSize = 60.f;
    Board board(tileSize);

    // Window layout: left side for move history, right side for board (centered)
    const float boardSize = tileSize * 8.f;  // 480.f
    const float historyPanelWidth = 300.f;   // space on the left for move history
    const float windowWidth = historyPanelWidth + boardSize + 40.f;  // 820.f (with margin)
    const float windowHeight = boardSize + 40.f;  // 520.f (with vertical margin)

    // Center board vertically, position it on the right with margin
    const float boardX = historyPanelWidth + 20.f;
    const float boardY = 20.f;
    board.setPosition({boardX, boardY});

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(static_cast<unsigned int>(windowWidth), static_cast<unsigned int>(windowHeight))), "Chess - SFML 3");
    window.setVerticalSyncEnabled(true);

    // Load piece style: prefer "maestro" if available, otherwise pick first available.
    std::string defaultStyle = "maestro";
    auto allStyles = PieceManager::listAvailableStyles("../assets/pieces");
    std::unique_ptr<PieceManager> pmPtr;
    int currentStyleIndex = 0;
    
    if (allStyles.empty()) {
        std::cerr << "No piece styles found in assets/pieces. Pieces will not be shown.\n";
    } else {
        // find index of default style
        for (size_t i = 0; i < allStyles.size(); ++i) {
            if (allStyles[i] == defaultStyle) {
                currentStyleIndex = i;
                break;
            }
        }

        pmPtr = std::make_unique<PieceManager>(allStyles[currentStyleIndex], "../assets/pieces");
        if (!pmPtr->isLoaded()) {
            std::cerr << "Failed to load piece style: " << allStyles[currentStyleIndex] << "\n";
            pmPtr.reset();
        } else {
            std::cout << "Loaded piece style: " << allStyles[currentStyleIndex] << "\n";
            board.setPieceManager(pmPtr.get());
            board.setStyle(allStyles[currentStyleIndex]);
            board.setInitialPosition();
        }
    }

    // Font for UI text (try system fonts)
    sf::Font font;
    if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "Warning: Could not load font. Style text will not be displayed.\n";
    }

    while (window.isOpen()) {
        while (auto evt = window.pollEvent()) {
            // SFML 3 uses an event object as a variant — helpers are provided to
            // query and fetch the subtype data.
            if (evt->is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            if (evt->is<sf::Event::KeyPressed>()) {
                const auto *key = evt->getIf<sf::Event::KeyPressed>();
                if (key) {
                    if (key->code == sf::Keyboard::Key::Escape) {
                        window.close();
                        break;
                    }
                    // Cycle through piece styles with Left/Right
                    if (key->code == sf::Keyboard::Key::Right && !allStyles.empty()) {
                        currentStyleIndex = (currentStyleIndex + 1) % allStyles.size();
                        pmPtr = std::make_unique<PieceManager>(allStyles[currentStyleIndex], "../assets/pieces");
                        if (pmPtr->isLoaded()) {
                            board.setPieceManager(pmPtr.get());
                            board.setStyle(allStyles[currentStyleIndex]);
                            board.setInitialPosition();
                            std::cout << "Switched to style: " << allStyles[currentStyleIndex] << "\n";
                        }
                    }
                    if (key->code == sf::Keyboard::Key::Left && !allStyles.empty()) {
                        currentStyleIndex = (currentStyleIndex + allStyles.size() - 1) % allStyles.size();
                        pmPtr = std::make_unique<PieceManager>(allStyles[currentStyleIndex], "../assets/pieces");
                        if (pmPtr->isLoaded()) {
                            board.setPieceManager(pmPtr.get());
                            board.setStyle(allStyles[currentStyleIndex]);
                            board.setInitialPosition();
                            std::cout << "Switched to style: " << allStyles[currentStyleIndex] << "\n";
                        }
                    }

                    // Cycle board color palettes with Up/Down
                    if (key->code == sf::Keyboard::Key::Up) {
                        board.cyclePalette(1);
                        auto cols = board.getColorsRGB();
                        auto &l = cols.first; auto &d = cols.second;
                        std::cout << "Board colors set to light=(" << l[0] << "," << l[1] << "," << l[2] << ") dark=(" << d[0] << "," << d[1] << "," << d[2] << ")\n";
                    }
                    if (key->code == sf::Keyboard::Key::Down) {
                        board.cyclePalette(-1);
                        auto cols = board.getColorsRGB();
                        auto &l = cols.first; auto &d = cols.second;
                        std::cout << "Board colors set to light=(" << l[0] << "," << l[1] << "," << l[2] << ") dark=(" << d[0] << "," << d[1] << "," << d[2] << ")\n";
                    }
                }
            }
        }

        window.clear(sf::Color(50, 50, 50));

        // Draw history panel background (left side)
        sf::RectangleShape historyPanel({historyPanelWidth, windowHeight});
        historyPanel.setPosition({0.f, 0.f});
        historyPanel.setFillColor(sf::Color(30, 30, 30));
        window.draw(historyPanel);

        // Draw separator line between history panel and board
        sf::RectangleShape separator({2.f, windowHeight});
        separator.setPosition({historyPanelWidth, 0.f});
        separator.setFillColor(sf::Color(100, 100, 100));
        window.draw(separator);

        // Draw current piece style info in history panel
        if (font.getInfo().family.size() > 0 && !allStyles.empty()) {
            sf::Text styleLabel(font, "Piece Style:", 14);
            styleLabel.setPosition({10.f, 20.f});
            styleLabel.setFillColor(sf::Color(200, 200, 200));
            window.draw(styleLabel);

            sf::Text styleName(font, allStyles[currentStyleIndex], 16);
            styleName.setPosition({10.f, 40.f});
            styleName.setFillColor(sf::Color(255, 200, 100));
            window.draw(styleName);

            sf::Text hints(font, "Use <- -> to change piece style", 11);
            hints.setPosition({10.f, 70.f});
            hints.setFillColor(sf::Color(150, 150, 150));
            window.draw(hints);

            // Show board colors and keyboard hint
            auto cols = board.getColorsRGB();
            Board::RGB l = cols.first; Board::RGB d = cols.second;
            char buf[128];
            std::snprintf(buf, sizeof(buf), "Board colors L:(%d,%d,%d) D:(%d,%d,%d)", l[0], l[1], l[2], d[0], d[1], d[2]);
            sf::Text colorInfo(font, buf, 12);
            colorInfo.setPosition({10.f, 92.f});
            colorInfo.setFillColor(sf::Color(200,200,200));
            window.draw(colorInfo);

            sf::Text colorHint(font, "Use Up/Down to change board colors", 11);
            colorHint.setPosition({10.f, 114.f});
            colorHint.setFillColor(sf::Color(150,150,150));
            window.draw(colorHint);
        }

        // Draw the board
        window.draw(board);
        window.display();
    }

    return 0;
}
