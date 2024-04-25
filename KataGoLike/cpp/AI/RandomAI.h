#pragma once

#include "AI.h"
#include <random>

class RandomAI : public AI
{
    std::pair<int, int> move(const GoGame& game) override;
};