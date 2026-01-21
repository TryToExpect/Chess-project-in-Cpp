#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include <SFML/Audio.hpp>

class sound_player {
public:
    explicit sound_player(std::filesystem::path baseDir);

    bool load(const std::string& audio_name, const std::string& audio_file);
    bool loadAuto(const std::string& audio_name);

    bool play(const std::string& audio_name, float volume = 100.0f);

    void stopAll();
    void stop(const std::string& audio_name);

    void setMasterVolume(float volume);
    float masterVolume() const;

    void update();

private:
    std::filesystem::path m_baseDir;
    std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
    std::vector<sf::Sound> m_sounds;

    float m_masterVolume = 100.0f;

    static float clampVolume(float v);
};
