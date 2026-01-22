#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include "Board.hpp"
#include "PieceManager.hpp"
#include "GameLogic.hpp"
#include "GameRecorder.hpp"
#include "SoundManager.hpp"
#include <memory>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

// Format seconds as MM:SS for the side clocks
static std::string formatClockTime(double seconds) {
    double clamped = std::max(0.0, seconds);
    int totalSeconds = static_cast<int>(clamped);
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << secs;
    return oss.str();
}

enum class GameState {
    MENU,
    PLAYING
};

struct TimeControl {
    std::string name;
    double seconds;
};

int main() {
    // Game state management
    GameState gameState = GameState::MENU;
    
    // Time control options
    std::vector<TimeControl> timeControls = {
        {"Bullet 1min", 60.0},
        {"Blitz 3min", 180.0},
        {"Blitz 5min", 300.0},
        {"Rapid 10min", 600.0},
        {"Rapid 15min", 900.0}
    };
    int selectedTimeControl = 2; // Default: Blitz 5min
    
    // Dynamic tile size — will scale based on window size
    float tileSize = 60.f;
    Board board(tileSize);
    GameLogic game;

    // Game recorder for saving moves to file
    GameRecorder gameRecorder;
    game.setGameRecorder(&gameRecorder);

    // Sound manager
    SoundManager soundManager;
    soundManager.loadSounds("../assets/sounds");
    
    // Setup sound callback for game logic
    game.setSoundCallback([&soundManager](bool isPawnMove, bool isCapture) {
        if (isPawnMove) {
            if (isCapture) {
                soundManager.playPawnHit();
            } else {
                soundManager.playPawnMove();
            }
        }
    });

    // Simple time control (per side, in seconds)
    double initialClockSeconds = timeControls[selectedTimeControl].seconds;
    double whiteTimeSeconds = initialClockSeconds;
    double blackTimeSeconds = initialClockSeconds;
    bool timeExpired = false;
    Color timeOutSide = Color::NONE;
    sf::Clock deltaClock;
    bool gameStarted = false; // Track if first move has been made

    // Window layout: left side for move history, right side for board (centered)
    float boardSize = tileSize * 8.f;  // Will be recalculated on resize
    const float historyPanelWidth = 300.f;   // space on the left for move history
    float windowWidth = historyPanelWidth + boardSize + 40.f;  // 820.f (with margin)
    float windowHeight = boardSize + 40.f;  // 520.f (with vertical margin)

    // Center board in the available space (right of history panel)
    float availableWidth = windowWidth - historyPanelWidth;
    float boardX = historyPanelWidth + (availableWidth - boardSize) / 2.f;
    float boardY = (windowHeight - boardSize) / 2.f;
    board.setPosition({boardX, boardY});

    // Mouse selection state
    int selectedRow = -1, selectedCol = -1;  // -1 means no selection
    
    // Drag-and-drop state
    bool isDragging = false;
    int dragStartRow = -1, dragStartCol = -1;
    sf::CircleShape dragPreview(0.f);  // Visual feedback for dragging

    // Pawn promotion state
    bool isPromotionPending = false;
    int promotionRow = -1, promotionCol = -1;
    Move promotionMove = {-1, -1, -1, -1};

    // Track if end sound has been played
    bool endSoundPlayed = false;

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

    // Don't start clock until game begins
    // deltaClock will be restarted when entering PLAYING state

    while (window.isOpen()) {
        while (auto evt = window.pollEvent()) {
            // SFML 3 uses an event object as a variant — helpers are provided to
            // query and fetch the subtype data.
            if (evt->is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            // Handle mouse button press for menu
            if (gameState == GameState::MENU && evt->is<sf::Event::MouseButtonPressed>()) {
                const auto* mouseBtn = evt->getIf<sf::Event::MouseButtonPressed>();
                if (mouseBtn && mouseBtn->button == sf::Mouse::Button::Left) {
                    float mx = static_cast<float>(mouseBtn->position.x);
                    float my = static_cast<float>(mouseBtn->position.y);
                    
                    // Menu layout centered
                    float menuX = windowWidth / 2.f - 150.f;
                    float menuY = 150.f;
                    float buttonWidth = 300.f;
                    float buttonHeight = 50.f;
                    float spacing = 60.f;
                    
                    // Check time control buttons
                    for (size_t i = 0; i < timeControls.size(); ++i) {
                        float btnY = menuY + i * spacing;
                        if (mx >= menuX && mx <= menuX + buttonWidth &&
                            my >= btnY && my <= btnY + buttonHeight) {
                            selectedTimeControl = i;
                        }
                    }
                    
                    // Check START button
                    float startBtnY = menuY + timeControls.size() * spacing + 30.f;
                    if (mx >= menuX && mx <= menuX + buttonWidth &&
                        my >= startBtnY && my <= startBtnY + buttonHeight) {
                        // Start game
                        gameState = GameState::PLAYING;
                        initialClockSeconds = timeControls[selectedTimeControl].seconds;
                        whiteTimeSeconds = initialClockSeconds;
                        blackTimeSeconds = initialClockSeconds;
                        deltaClock.restart(); // Start timing now
                        soundManager.playBackgroundMusic(); // Start background music
                        std::cout << "Game started with time control: " << timeControls[selectedTimeControl].name << "\n";
                    }
                }
            }

            // Handle window resize
            if (evt->is<sf::Event::Resized>()) {
                const auto* resized = evt->getIf<sf::Event::Resized>();
                if (resized) {
                    windowWidth = static_cast<float>(resized->size.x);
                    windowHeight = static_cast<float>(resized->size.y);
                    
                    // Recalculate tile size based on available space
                    // Leave margins: 20px on each side of the board
                    float availableWidth = windowWidth - historyPanelWidth - 40.f;   // 40 = 20px margins
                    float availableHeight = windowHeight - 40.f;                      // 40 = 20px margins
                    
                    // Tile size must fit 8x8 board in available space
                    tileSize = std::min(availableWidth / 8.f, availableHeight / 8.f);
                    board.setTileSize(tileSize);
                    
                    // Update board size and position
                    boardSize = tileSize * 8.f;
                    availableWidth = windowWidth - historyPanelWidth;
                    boardX = historyPanelWidth + (availableWidth - boardSize) / 2.f;
                    boardY = (windowHeight - boardSize) / 2.f;
                    board.setPosition({boardX, boardY});
                    
                    // Update camera view to match new window size
                    sf::View view(sf::Vector2f(windowWidth / 2.f, windowHeight / 2.f), 
                                 sf::Vector2f(windowWidth, windowHeight));
                    window.setView(view);
                }
            }

            // Handle mouse button press (start drag or promotion choice)
            if (gameState == GameState::PLAYING && evt->is<sf::Event::MouseButtonPressed>()) {
                const auto* mouseBtn = evt->getIf<sf::Event::MouseButtonPressed>();
                if (mouseBtn && mouseBtn->button == sf::Mouse::Button::Left) {
                    float mx = static_cast<float>(mouseBtn->position.x);
                    float my = static_cast<float>(mouseBtn->position.y);

                    // If promotion is pending, check for promotion button clicks
                    if (isPromotionPending && !timeExpired) {
                        float popupCenterX = boardX + promotionCol * tileSize + tileSize / 2.f;
                        float popupCenterY = boardY + promotionRow * tileSize + tileSize / 2.f;
                        float buttonSize = 35.f;
                        float spacing = 50.f;

                        // Knight button
                        if (mx >= popupCenterX - spacing - buttonSize/2 && mx <= popupCenterX - spacing + buttonSize/2 &&
                            my >= popupCenterY - buttonSize/2 && my <= popupCenterY + buttonSize/2) {
                            promotionMove.promotionPiece = PieceType::KNIGHT;
                            game.makeMove(promotionMove);
                            gameStarted = true; // Start timing on first move
                            board.updateFromGame(game);
                            moveHistory.push_back("promotion to Knight");
                            isPromotionPending = false;
                            std::cout << "Promoted to Knight\n";
                        }
                        // Bishop button
                        else if (mx >= popupCenterX - buttonSize/2 && mx <= popupCenterX + buttonSize/2 &&
                                my >= popupCenterY - spacing - buttonSize/2 && my <= popupCenterY - spacing + buttonSize/2) {
                            promotionMove.promotionPiece = PieceType::BISHOP;
                            game.makeMove(promotionMove);
                            gameStarted = true; // Start timing on first move
                            board.updateFromGame(game);
                            moveHistory.push_back("promotion to Bishop");
                            isPromotionPending = false;
                            std::cout << "Promoted to Bishop\n";
                        }
                        // Rook button
                        else if (mx >= popupCenterX + spacing - buttonSize/2 && mx <= popupCenterX + spacing + buttonSize/2 &&
                                my >= popupCenterY - buttonSize/2 && my <= popupCenterY + buttonSize/2) {
                            promotionMove.promotionPiece = PieceType::ROOK;
                            game.makeMove(promotionMove);
                            gameStarted = true; // Start timing on first move
                            board.updateFromGame(game);
                            moveHistory.push_back("promotion to Rook");
                            isPromotionPending = false;
                            std::cout << "Promoted to Rook\n";
                        }
                        // Queen button
                        else if (mx >= popupCenterX - buttonSize/2 && mx <= popupCenterX + buttonSize/2 &&
                                my >= popupCenterY + spacing - buttonSize/2 && my <= popupCenterY + spacing + buttonSize/2) {
                            promotionMove.promotionPiece = PieceType::QUEEN;
                            game.makeMove(promotionMove);
                            gameStarted = true; // Start timing on first move
                            board.updateFromGame(game);
                            moveHistory.push_back("promotion to Queen");
                            isPromotionPending = false;
                            std::cout << "Promoted to Queen\n";
                        }
                    }
                    // Normal piece dragging
                    else if (!timeExpired && mx >= boardX && mx < boardX + boardSize && my >= boardY && my < boardY + boardSize) {
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
            if (gameState == GameState::PLAYING && evt->is<sf::Event::MouseButtonReleased>()) {
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
                                        // Check for pawn promotion
                                        if (piece->type == PieceType::PAWN && (row == 0 || row == 7)) {
                                            m.isPromotion = true;
                                            isPromotionPending = true;
                                            promotionRow = row;
                                            promotionCol = col;
                                            promotionMove = m;
                                            std::cout << "Pawn promotion required at " << static_cast<char>('a' + col) << (8 - row) << "\n";
                                            dragStartRow = -1;
                                            dragStartCol = -1;
                                        } else {
                                            // Normal move (no promotion)
                                            game.makeMove(m);
                                            gameStarted = true; // Start timing on first move

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
                                                // Save game with result
                                                GameResult result = (game.getWinner() == Color::WHITE) ? GameResult::WHITE_WIN_CHECKMATE : GameResult::BLACK_WIN_CHECKMATE;
                                                game.endGameWithResult(result, "checkmate");
                                                if (!endSoundPlayed) {
                                                    soundManager.playEndSound();
                                                    endSoundPlayed = true;
                                                }
                                            } else if (game.isStalemate()) {
                                                std::cout << "STALEMATE - Draw!\n";
                                                game.endGameWithResult(GameResult::STALEMATE, "stalemate");
                                                if (!endSoundPlayed) {
                                                    soundManager.playEndSound();
                                                    endSoundPlayed = true;
                                                }
                                            } else if (game.isInCheck(game.getTurn())) {
                                                std::cout << "CHECK!\n";
                                            }

                                            dragStartRow = -1;
                                            dragStartCol = -1;
                                        }
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
            if (gameState == GameState::PLAYING && evt->is<sf::Event::MouseMoved>()) {
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

                    // Toggle sound with M key
                    if (key->code == sf::Keyboard::Key::M) {
                        soundManager.toggleSound();
                    }
                }
            }
        }

        // Advance active side clock (stop when a side flags or the game ends)
        float deltaSeconds = deltaClock.restart().asSeconds();
        if (gameState == GameState::PLAYING && gameStarted && !game.isGameOver() && !timeExpired) {
            double delta = static_cast<double>(deltaSeconds);
            if (game.getTurn() == Color::WHITE) {
                whiteTimeSeconds = std::max(0.0, whiteTimeSeconds - delta);
                if (whiteTimeSeconds <= 0.0) {
                    timeExpired = true;
                    timeOutSide = Color::WHITE;
                    if (!endSoundPlayed) {
                        soundManager.playEndSound();
                        endSoundPlayed = true;
                    }
                }
            } else {
                blackTimeSeconds = std::max(0.0, blackTimeSeconds - delta);
                if (blackTimeSeconds <= 0.0) {
                    timeExpired = true;
                    timeOutSide = Color::BLACK;
                    if (!endSoundPlayed) {
                        soundManager.playEndSound();
                        endSoundPlayed = true;
                    }
                }
            }
        }

        window.clear(sf::Color(50, 50, 50));

        if (gameState == GameState::MENU) {
            // Draw menu
            if (font.getInfo().family.size() > 0) {
                // Title
                sf::Text title(font, "CHESS", 48);
                title.setPosition({windowWidth / 2.f - 100.f, 50.f});
                title.setFillColor(sf::Color(255, 255, 255));
                window.draw(title);

                sf::Text subtitle(font, "Select Time Control", 24);
                subtitle.setPosition({windowWidth / 2.f - 130.f, 110.f});
                subtitle.setFillColor(sf::Color(200, 200, 200));
                window.draw(subtitle);

                // Time control buttons
                float menuX = windowWidth / 2.f - 150.f;
                float menuY = 150.f;
                float buttonWidth = 300.f;
                float buttonHeight = 50.f;
                float spacing = 60.f;

                for (size_t i = 0; i < timeControls.size(); ++i) {
                    float btnY = menuY + i * spacing;
                    
                    sf::RectangleShape button({buttonWidth, buttonHeight});
                    button.setPosition({menuX, btnY});
                    
                    if (static_cast<int>(i) == selectedTimeControl) {
                        button.setFillColor(sf::Color(80, 120, 180));
                        button.setOutlineThickness(3.f);
                        button.setOutlineColor(sf::Color(120, 180, 255));
                    } else {
                        button.setFillColor(sf::Color(60, 60, 80));
                        button.setOutlineThickness(2.f);
                        button.setOutlineColor(sf::Color(100, 100, 120));
                    }
                    window.draw(button);

                    sf::Text btnText(font, timeControls[i].name, 20);
                    btnText.setPosition({menuX + 20.f, btnY + 12.f});
                    btnText.setFillColor(sf::Color(255, 255, 255));
                    window.draw(btnText);
                }

                // START button
                float startBtnY = menuY + timeControls.size() * spacing + 30.f;
                sf::RectangleShape startButton({buttonWidth, buttonHeight});
                startButton.setPosition({menuX, startBtnY});
                startButton.setFillColor(sf::Color(50, 150, 50));
                startButton.setOutlineThickness(3.f);
                startButton.setOutlineColor(sf::Color(100, 200, 100));
                window.draw(startButton);

                sf::Text startText(font, "START GAME", 24);
                startText.setPosition({menuX + 70.f, startBtnY + 10.f});
                startText.setFillColor(sf::Color(255, 255, 255));
                window.draw(startText);

                // Sound toggle info
                sf::Text soundInfo(font, std::string("Sound: ") + (soundManager.isSoundEnabled() ? "ON (Press M to toggle)" : "OFF (Press M to toggle)"), 14);
                soundInfo.setPosition({menuX + 10.f, startBtnY + 70.f});
                soundInfo.setFillColor(soundManager.isSoundEnabled() ? sf::Color(150, 255, 150) : sf::Color(255, 150, 150));
                window.draw(soundInfo);
            }
        } else {
            // Draw game (existing code)
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

            if (timeExpired) {
                std::string loser = (timeOutSide == Color::WHITE) ? "White" : "Black";
                sf::Text timeLabel(font, loser + " out of time", 14);
                timeLabel.setPosition({10.f, 90.f});
                timeLabel.setFillColor(sf::Color(255, 120, 120));
                window.draw(timeLabel);
            }

            // Clocks for both sides
            const float clockWidth = historyPanelWidth - 20.f;
            const float clockHeight = 50.f;
            auto drawClock = [&](const std::string& label, double seconds, float y, bool isActive, const sf::Color& faceColor) {
                sf::RectangleShape clockShape({clockWidth, clockHeight});
                clockShape.setPosition({10.f, y});
                clockShape.setFillColor(faceColor);
                clockShape.setOutlineThickness(isActive ? 3.f : 1.5f);
                clockShape.setOutlineColor(isActive ? sf::Color(120, 180, 255) : sf::Color(90, 90, 90));
                window.draw(clockShape);

                sf::Text labelText(font, label, 12);
                labelText.setPosition({clockShape.getPosition().x + 10.f, y + 8.f});
                labelText.setFillColor(sf::Color(40, 40, 40));
                window.draw(labelText);

                sf::Text timeText(font, formatClockTime(seconds), 22);
                timeText.setPosition({clockShape.getPosition().x + clockWidth - 90.f, y + 10.f});
                timeText.setFillColor(sf::Color(20, 20, 20));
                window.draw(timeText);
            };

            bool whiteActive = (game.getTurn() == Color::WHITE) && !game.isGameOver() && !timeExpired;
            bool blackActive = (game.getTurn() == Color::BLACK) && !game.isGameOver() && !timeExpired;
            drawClock("White", whiteTimeSeconds, 110.f, whiteActive, sf::Color(230, 230, 240));
            drawClock("Black", blackTimeSeconds, 170.f, blackActive, sf::Color(60, 60, 80));

            // Controls
            sf::Text controlsLabel(font, "Controls:", 12);
            controlsLabel.setPosition({10.f, 240.f});
            controlsLabel.setFillColor(sf::Color(200, 200, 200));
            window.draw(controlsLabel);

            sf::Text controls1(font, "Click & drag to move", 10);
            controls1.setPosition({10.f, 260.f});
            controls1.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls1);

            sf::Text controls2(font, "R: Cancel drag", 10);
            controls2.setPosition({10.f, 275.f});
            controls2.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls2);

            sf::Text controls3(font, "Left/Right: Styles", 10);
            controls3.setPosition({10.f, 290.f});
            controls3.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls3);

            sf::Text controls4(font, "Up/Down: Colors", 10);
            controls4.setPosition({10.f, 305.f});
            controls4.setFillColor(sf::Color(150, 150, 150));
            window.draw(controls4);

            sf::Text controls5(font, std::string("M: Sound ") + (soundManager.isSoundEnabled() ? "ON" : "OFF"), 10);
            controls5.setPosition({10.f, 320.f});
            controls5.setFillColor(soundManager.isSoundEnabled() ? sf::Color(150, 255, 150) : sf::Color(255, 150, 150));
            window.draw(controls5);

            // Move history
            sf::Text historyLabel(font, "Moves:", 12);
            historyLabel.setPosition({10.f, 350.f});
            historyLabel.setFillColor(sf::Color(200, 200, 200));
            window.draw(historyLabel);

            int moveY = 370;
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

        // Draw promotion popup if pending
        if (isPromotionPending) {
            float popupCenterX = boardX + promotionCol * tileSize + tileSize / 2.f;
            float popupCenterY = boardY + promotionRow * tileSize + tileSize / 2.f;
            float buttonSize = 35.f;
            float spacing = 50.f;

            // Semi-transparent overlay
            sf::RectangleShape overlay({boardSize, boardSize});
            overlay.setPosition({boardX, boardY});
            overlay.setFillColor(sf::Color(0, 0, 0, 100));
            window.draw(overlay);

            // Title
            if (font.getInfo().family.size() > 0) {
                sf::Text promoteLabel(font, "Choose promotion:", 14);
                promoteLabel.setPosition({popupCenterX - 60.f, popupCenterY - 80.f});
                promoteLabel.setFillColor(sf::Color(255, 255, 255));
                window.draw(promoteLabel);
            }

            // Knight button (left)
            sf::RectangleShape knightBtn({buttonSize, buttonSize});
            knightBtn.setPosition({popupCenterX - spacing - buttonSize/2.f, popupCenterY - buttonSize/2.f});
            knightBtn.setFillColor(sf::Color(100, 150, 200));
            window.draw(knightBtn);
            if (font.getInfo().family.size() > 0) {
                sf::Text knightLabel(font, "N", 16);
                knightLabel.setPosition({popupCenterX - spacing - 8.f, popupCenterY - 12.f});
                knightLabel.setFillColor(sf::Color(255, 255, 255));
                window.draw(knightLabel);
            }

            // Bishop button (top)
            sf::RectangleShape bishopBtn({buttonSize, buttonSize});
            bishopBtn.setPosition({popupCenterX - buttonSize/2.f, popupCenterY - spacing - buttonSize/2.f});
            bishopBtn.setFillColor(sf::Color(150, 100, 200));
            window.draw(bishopBtn);
            if (font.getInfo().family.size() > 0) {
                sf::Text bishopLabel(font, "B", 16);
                bishopLabel.setPosition({popupCenterX - 8.f, popupCenterY - spacing - 12.f});
                bishopLabel.setFillColor(sf::Color(255, 255, 255));
                window.draw(bishopLabel);
            }

            // Rook button (right)
            sf::RectangleShape rookBtn({buttonSize, buttonSize});
            rookBtn.setPosition({popupCenterX + spacing - buttonSize/2.f, popupCenterY - buttonSize/2.f});
            rookBtn.setFillColor(sf::Color(200, 150, 100));
            window.draw(rookBtn);
            if (font.getInfo().family.size() > 0) {
                sf::Text rookLabel(font, "R", 16);
                rookLabel.setPosition({popupCenterX + spacing - 8.f, popupCenterY - 12.f});
                rookLabel.setFillColor(sf::Color(255, 255, 255));
                window.draw(rookLabel);
            }

            // Queen button (bottom)
            sf::RectangleShape queenBtn({buttonSize, buttonSize});
            queenBtn.setPosition({popupCenterX - buttonSize/2.f, popupCenterY + spacing - buttonSize/2.f});
            queenBtn.setFillColor(sf::Color(200, 200, 100));
            window.draw(queenBtn);
            if (font.getInfo().family.size() > 0) {
                sf::Text queenLabel(font, "Q", 16);
                queenLabel.setPosition({popupCenterX - 8.f, popupCenterY + spacing - 12.f});
                queenLabel.setFillColor(sf::Color(0, 0, 0));
                window.draw(queenLabel);
            }
        }
        } // End of gameState == PLAYING

        window.display();
    }

    return 0;
}
