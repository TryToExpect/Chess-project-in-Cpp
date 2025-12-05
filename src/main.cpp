#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include "Board.hpp"

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
                if (key && key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                    break;
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

        // Draw the board
        window.draw(board);
        window.display();
    }

    return 0;
}
