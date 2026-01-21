#include "sound_player.hpp"

#include <algorithm>
#include <iostream>
#include <utility>

sound_player::sound_player(std::filesystem::path baseDir)
    : m_baseDir(std::move(baseDir)) {}

float sound_player::clampVolume(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 100.0f) return 100.0f;
    return v;
}

bool sound_player::load(const std::string& audio_name, const std::string& audio_file) {
    const auto fullPath = m_baseDir / audio_file;

    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(fullPath.string())) {
        std::cerr << "[SoundPlayer] Nie moge wczytac: " << fullPath << "\n";
        return false;
    }


    m_buffers[audio_name] = std::move(buffer);
    return true;
}

bool sound_player::loadAuto(const std::string& audio_name) {
    return load(audio_name, audio_name + ".wav");
}

bool sound_player::play(const std::string& audio_name, float volume) {
    auto it = m_buffers.find(audio_name);
    if (it == m_buffers.end()) {
        std::cerr << "[SoundPlayer] Brak zaladowanego dzwieku: " << audio_name << "\n";
        return false;
    }

    update();

    m_sounds.emplace_back(it->second);
    sf::Sound& s = m_sounds.back();

    const float finalVol =
        clampVolume((clampVolume(volume) * clampVolume(m_masterVolume)) / 100.0f);

    s.setVolume(finalVol);
    s.play();

    return true;
}

void sound_player::stopAll() {
    for (auto& s : m_sounds) {
        s.stop();
    }
    m_sounds.clear();
}

void sound_player::stop(const std::string& audio_name) {
    auto it = m_buffers.find(audio_name);
    if (it == m_buffers.end()) return;

    const sf::SoundBuffer& target = it->second;

    for (auto& s : m_sounds) {

        if (&s.getBuffer() == &target) {
            s.stop();
        }
    }

    update();
}

void sound_player::setMasterVolume(float volume) {
    m_masterVolume = clampVolume(volume);
}

float sound_player::masterVolume() const {
    return m_masterVolume;
}

void sound_player::update() {
    const auto newEnd = std::remove_if(
        m_sounds.begin(),
        m_sounds.end(),
        [](const sf::Sound& s) {
            return s.getStatus() == sf::SoundSource::Status::Stopped;
        }
    );

    m_sounds.erase(newEnd, m_sounds.end());
}