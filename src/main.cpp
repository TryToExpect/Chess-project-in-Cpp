#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include "Board.hpp"
#include "PieceManager.hpp"
#include "GameLogic.hpp"
#include <memory>
#include <iostream>
#include <vector>

int main() {
    // Fixed tile size — board won't scale on window resize
    const float tileSize = 60.f;
    Board board(tileSize);
    GameLogic game;

    // Window layout: left side for move history, right side for board (centered)
    const float boardSize = tileSize * 8.f;  // 480.f
    const float historyPanelWidth = 300.f;   // space on the left for move history
    const float windowWidth = historyPanelWidth + boardSize + 40.f;  // 820.f (with margin)
    const float windowHeight = boardSize + 40.f;  // 520.f (with vertical margin)

    // Center board vertically, position it on the right with margin
    const float boardX = historyPanelWidth + 20.f;
    const float boardY = 20.f;
    board.setPosition({boardX, boardY});

    // Mouse selection state
    int selectedRow = -1, selectedCol = -1;  // -1 means no selection
    
    // Drag-and-drop state
    bool isDragging = false;
    int dragStartRow = -1, dragStartCol = -1;
    sf::CircleShape dragPreview(0.f);  // Visual feedback for dragging

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
            board.updateFromGame(game);  // Initialize board from game state
        }
    }

    // Font for UI text (try system fonts)
    sf::Font font;
    if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "Warning: Could not load font. Style text will not be displayed.\n";
    }

    // Move history
    std::vector<std::string> moveHistory;

    while (window.isOpen()) {
        while (auto evt = window.pollEvent()) {
            // SFML 3 uses an event object as a variant — helpers are provided to
            // query and fetch the subtype data.
            if (evt->is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            // Handle mouse button press (start drag)
            if (evt->is<sf::Event::MouseButtonPressed>()) {
                const auto* mouseBtn = evt->getIf<sf::Event::MouseButtonPressed>();
                if (mouseBtn && mouseBtn->button == sf::Mouse::Button::Left) {
                    float mx = static_cast<float>(mouseBtn->position.x);
                    float my = static_cast<float>(mouseBtn->position.y);

                    // Check if click is within board bounds
                    if (mx >= boardX && mx < boardX + boardSize && my >= boardY && my < boardY + boardSize) {
                        int col = static_cast<int>((mx - boardX) / tileSize);
                        int row = static_cast<int>((my - boardY) / tileSize);

                        // Validate bounds
                        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                            Piece* clickedPiece = game.getPiece(row, col);

                            // Start drag only if clicking on own piece
                            if (clickedPiece && clickedPiece->color == game.getTurn()) {
                                isDragging = true;
                                dragStartRow = row;
                                dragStartCol = col;
                                dragPreview.setRadius(tileSize / 2.f);
                                dragPreview.setFillColor(sf::Color(100, 200, 100, 150));
                                std::cout << "Dragging piece from: " << static_cast<char>('a' + col) << (8 - row) << "\n";
                            }
                        }
                    }
                }
            }

            // Handle mouse button release (end drag and execute move)
            if (evt->is<sf::Event::MouseButtonReleased>()) {
                const auto* mouseBtn = evt->getIf<sf::Event::MouseButtonReleased>();
                if (mouseBtn && mouseBtn->button == sf::Mouse::Button::Left && isDragging) {
                    isDragging = false;
                    
                    float mx = static_cast<float>(mouseBtn->position.x);
                    float my = static_cast<float>(mouseBtn->position.y);

                    // Check if release is within board bounds
                    if (mx >= boardX && mx < boardX + boardSize && my >= boardY && my < boardY + boardSize) {
                        int col = static_cast<int>((mx - boardX) / tileSize);
                        int row = static_cast<int>((my - boardY) / tileSize);

                        // Validate bounds
                        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                            // Same square - deselect or highlight
                            if (dragStartRow == row && dragStartCol == col) {
                                std::cout << "Piece released on same square.\n";
                            } else {
                                // Attempt move
                                Move m = {dragStartRow, dragStartCol, row, col};
                                Piece* piece = game.getPiece(dragStartRow, dragStartCol);

                                if (piece && piece->isPseudoLegal(dragStartRow, dragStartCol, row, col, game)) {
                                    // Check for en passant
                                    if (piece->type == PieceType::PAWN && std::abs(dragStartCol - col) == 1 && 
                                        game.getPiece(row, col) == nullptr) {
                                        m.isEnPassant = true;
                                    }

                                    // Check for castling
                                    if (piece->type == PieceType::KING && std::abs(dragStartCol - col) == 2) {
                                        m.isCastling = true;
                                    }

                                    // Check path for sliders
                                    if (piece->type == PieceType::ROOK || piece->type == PieceType::BISHOP || piece->type == PieceType::QUEEN) {
                                        if (!game.isPathClear(dragStartRow, dragStartCol, row, col)) {
                                            std::cout << "Error: Path is blocked!\n";
                                            dragStartRow = -1;
                                            dragStartCol = -1;
                                            continue;
                                        }
                                    }

                                    // Simulate move to check king safety
                                    if (game.tryMove(m)) {
                                        game.makeMove(m);

                                        // Update board display from game state
                                        board.updateFromGame(game);

                                        // NO FLIP - keeps board orientation consistent

                                        // Add to history
                                        char colChar = 'a' + dragStartCol;
                                        char toCol = 'a' + col;
                                        moveHistory.push_back(std::string(1, colChar) + std::to_string(8 - dragStartRow) + 
                                                             std::string(1, toCol) + std::to_string(8 - row));

                                        std::cout << "Move: " << static_cast<char>('a' + dragStartCol) << (8 - dragStartRow) 
                                                  << " to " << static_cast<char>('a' + col) << (8 - row) << "\n";

                                        if (game.isCheckmate()) {
                                            std::cout << "CHECKMATE! " << (game.getWinner() == Color::WHITE ? "White" : "Black") << " wins!\n";
                                        } else if (game.isStalemate()) {
                                            std::cout << "STALEMATE - Draw!\n";
                                        } else if (game.isInCheck(game.getTurn())) {
                                            std::cout << "CHECK!\n";
                                        }

                                        dragStartRow = -1;
                                        dragStartCol = -1;
                                    } else {
                                        std::cout << "Illegal Move! Your King would be in check.\n";
                                        dragStartRow = -1;
                                        dragStartCol = -1;
                                    }
                                } else {
                                    std::cout << "Error: Invalid move for this piece type.\n";
                                    dragStartRow = -1;
                                    dragStartCol = -1;
                                }
                            }
                        }
                    }
                }
            }

            // Handle mouse move (for drag preview)
            if (evt->is<sf::Event::MouseMoved>()) {
                const auto* mouseMov = evt->getIf<sf::Event::MouseMoved>();
                if (mouseMov && isDragging) {
                    float mx = static_cast<float>(mouseMov->position.x);
                    float my = static_cast<float>(mouseMov->position.y);
                    dragPreview.setPosition({mx - dragPreview.getRadius(), my - dragPreview.getRadius()});
                }
            }

            if (evt->is<sf::Event::KeyPressed>()) {
                const auto *key = evt->getIf<sf::Event::KeyPressed>();
                if (key) {
                    if (key->code == sf::Keyboard::Key::Escape) {
                        window.close();
                        break;
                    }
                    // Cancel drag with R key
                    if (key->code == sf::Keyboard::Key::R) {
                        isDragging = false;
                        dragStartRow = -1;
                        dragStartCol = -1;
                        std::cout << "Drag cancelled.\n";
                    }
                    // Cycle through piece styles with Left/Right
                    if (key->code == sf::Keyboard::Key::Right && !allStyles.empty()) {
                        currentStyleIndex = (currentStyleIndex + 1) % allStyles.size();
                        pmPtr = std::make_unique<PieceManager>(allStyles[currentStyleIndex], "../assets/pieces");
                        if (pmPtr->isLoaded()) {
                            board.setPieceManager(pmPtr.get());
                            board.setStyle(allStyles[currentStyleIndex]);
                            std::cout << "Switched to style: " << allStyles[currentStyleIndex] << "\n";
                        }
                    }
                    if (key->code == sf::Keyboard::Key::Left && !allStyles.empty()) {
                        currentStyleIndex = (currentStyleIndex + allStyles.size() - 1) % allStyles.size();
                        pmPtr = std::make_unique<PieceManager>(allStyles[currentStyleIndex], "../assets/pieces");
                        if (pmPtr->isLoaded()) {
                            board.setPieceManager(pmPtr.get());
                            board.setStyle(allStyles[currentStyleIndex]);
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

        // Draw info and move history in history panel
        if (font.getInfo().family.size() > 0) {
            sf::Text titleLabel(font, "CHESS", 20);
            titleLabel.setPosition({10.f, 10.f});
            titleLabel.setFillColor(sf::Color(255, 255, 255));
            window.draw(titleLabel);

            // Current player and game status
            std::string currentPlayer = (game.getTurn() == Color::WHITE) ? "White" : "Black";
            sf::Text turnLabel(font, "Turn: " + currentPlayer, 14);
            turnLabel.setPosition({10.f, 40.f});
            turnLabel.setFillColor(game.getTurn() == Color::WHITE ? sf::Color(200, 200, 255) : sf::Color(100, 100, 100));
            window.draw(turnLabel);

            // Game status
            if (game.isCheckmate()) {
                std::string winner = (game.getWinner() == Color::WHITE) ? "White" : "Black";
                sf::Text statusLabel(font, "CHECKMATE!\n" + winner + " wins!", 14);
                statusLabel.setPosition({10.f, 65.f});
                statusLabel.setFillColor(sf::Color(255, 100, 100));
                window.draw(statusLabel);
            } else if (game.isStalemate()) {
                sf::Text statusLabel(font, "STALEMATE\nDRAW!", 14);
                statusLabel.setPosition({10.f, 65.f});
                statusLabel.setFillColor(sf::Color(255, 200, 100));
                window.draw(statusLabel);
            } else if (game.isInCheck(game.getTurn())) {
                sf::Text statusLabel(font, "CHECK!", 14);
                statusLabel.setPosition({10.f, 65.f});
                statusLabel.setFillColor(sf::Color(255, 150, 0));
                window.draw(statusLabel);
            }

            // Controls
            sf::Text controlsLabel(font, "Controls:", 12);
            controlsLabel.setPosition({10.f, 120.f});
            controlsLabel.setFillColor(sf::Color(200, 200, 200));
            window.draw(controlsLabel);

            sf::Text controls1(font, "Click & drag to move", 10);
            controls1.setPosition({10.f, 140.f});
            controls1.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls1);

            sf::Text controls2(font, "R: Cancel drag", 10);
            controls2.setPosition({10.f, 155.f});
            controls2.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls2);

            sf::Text controls3(font, "Left/Right: Styles", 10);
            controls3.setPosition({10.f, 170.f});
            controls3.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls3);

            sf::Text controls4(font, "Up/Down: Colors", 10);
            controls4.setPosition({10.f, 185.f});
            controls4.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls4);

            // Move history
            sf::Text historyLabel(font, "Moves:", 12);
            historyLabel.setPosition({10.f, 210.f});
            historyLabel.setFillColor(sf::Color(200, 200, 200));
            window.draw(historyLabel);

            int moveY = 230;
            for (size_t i = 0; i < moveHistory.size() && i < 12; i++) {
                std::string moveNum = std::to_string(i / 2 + 1) + ". " + moveHistory[i];
                sf::Text moveText(font, moveNum, 10);
                moveText.setPosition({10.f, static_cast<float>(moveY)});
                moveText.setFillColor(sf::Color(180, 180, 180));
                window.draw(moveText);
                moveY += 15;
            }

            if (moveHistory.size() > 12) {
                sf::Text moreText(font, "...", 10);
                moreText.setPosition({10.f, static_cast<float>(moveY)});
                moreText.setFillColor(sf::Color(100, 100, 100));
                window.draw(moreText);
            }
        }

        // Draw the board
        window.draw(board);

        // Draw drag preview circle while dragging
        if (isDragging && dragPreview.getRadius() > 0) {
            window.draw(dragPreview);
        }

        window.display();
    }

    return 0;
}
