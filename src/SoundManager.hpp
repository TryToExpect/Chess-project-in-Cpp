#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <memory>

class SoundManager {
public:
    SoundManager();
    ~SoundManager() = default;

    // Load all sound files
    bool loadSounds(const std::string& soundsPath);

    // Play sound effects
    void playPawnMove();
    void playPawnHit();
    void playEndSound();

    // Background music controls
    void playBackgroundMusic();
    void stopBackgroundMusic();

    // Toggle sound on/off
    void toggleSound();
    bool isSoundEnabled() const { return soundEnabled; }
    void setSoundEnabled(bool enabled);

private:
    // Sound buffers (hold audio data)
    sf::SoundBuffer pawnMoveBuffer;
    sf::SoundBuffer pawnHitBuffer;
    sf::SoundBuffer endSoundBuffer;

    // Sound objects (for playing effects) - using optional to handle SFML 3 requirements
    std::unique_ptr<sf::Sound> pawnMoveSound;
    std::unique_ptr<sf::Sound> pawnHitSound;
    std::unique_ptr<sf::Sound> endSound;

    // Background music
    sf::Music backgroundMusic;

    // Sound state
    bool soundEnabled;
    bool loaded;
};
