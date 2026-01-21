#include "SoundManager.hpp"
#include <iostream>

SoundManager::SoundManager() : soundEnabled(true), loaded(false) {}

bool SoundManager::loadSounds(const std::string& soundsPath) {
    // Load pawn move sound
    if (!pawnMoveBuffer.loadFromFile(soundsPath + "/pawn_move.wav")) {
        std::cerr << "Warning: Could not load pawn_move.wav\n";
        return false;
    }
    pawnMoveSound = std::make_unique<sf::Sound>(pawnMoveBuffer);

    // Load pawn hit sound
    if (!pawnHitBuffer.loadFromFile(soundsPath + "/pawn_hit.wav")) {
        std::cerr << "Warning: Could not load pawn_hit.wav\n";
        return false;
    }
    pawnHitSound = std::make_unique<sf::Sound>(pawnHitBuffer);

    // Load end sound
    if (!endSoundBuffer.loadFromFile(soundsPath + "/end_sound.wav")) {
        std::cerr << "Warning: Could not load end_sound.wav\n";
        return false;
    }
    endSound = std::make_unique<sf::Sound>(endSoundBuffer);

    // Load background music
    if (!backgroundMusic.openFromFile(soundsPath + "/background_sound.wav")) {
        std::cerr << "Warning: Could not load background_sound.wav\n";
        return false;
    }
    backgroundMusic.setLooping(true);  // Loop background music (SFML 3)
    backgroundMusic.setVolume(30.f); // Lower volume for background (0-100)

    loaded = true;
    std::cout << "Sound system loaded successfully.\n";
    return true;
}

void SoundManager::playPawnMove() {
    if (soundEnabled && loaded && pawnMoveSound) {
        pawnMoveSound->play();
    }
}

void SoundManager::playPawnHit() {
    if (soundEnabled && loaded && pawnHitSound) {
        pawnHitSound->play();
    }
}

void SoundManager::playEndSound() {
    if (soundEnabled && loaded && endSound) {
        endSound->play();
    }
}

void SoundManager::playBackgroundMusic() {
    if (soundEnabled && loaded && backgroundMusic.getStatus() != sf::Music::Status::Playing) {
        backgroundMusic.play();
    }
}

void SoundManager::stopBackgroundMusic() {
    if (backgroundMusic.getStatus() == sf::Music::Status::Playing) {
        backgroundMusic.stop();
    }
}

void SoundManager::toggleSound() {
    soundEnabled = !soundEnabled;
    
    if (!soundEnabled) {
        // Stop all sounds when disabled
        if (pawnMoveSound) pawnMoveSound->stop();
        if (pawnHitSound) pawnHitSound->stop();
        if (endSound) endSound->stop();
        stopBackgroundMusic();
    } else {
        // Restart background music when re-enabled
        playBackgroundMusic();
    }
    
    std::cout << "Sound " << (soundEnabled ? "enabled" : "disabled") << "\n";
}

void SoundManager::setSoundEnabled(bool enabled) {
    if (soundEnabled != enabled) {
        soundEnabled = enabled;
        
        if (!soundEnabled) {
            if (pawnMoveSound) pawnMoveSound->stop();
            if (pawnHitSound) pawnHitSound->stop();
            if (endSound) endSound->stop();
            stopBackgroundMusic();
        } else {
            playBackgroundMusic();
        }
    }
}
