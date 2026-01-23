#pragma once

#include "GameLogic.hpp"
#include <optional>

class Bot {
public:
    explicit Bot(Color botColor);

    std::optional<Move> pickMove(const GameLogic& game) const;

private:
    Color color_;
};

