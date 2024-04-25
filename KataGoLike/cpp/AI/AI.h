#pragma once

#include "../GoGame/GoGame.h"

class AI
{
public:
    virtual std::pair<int, int> move(const GoGame& game) = 0;
    virtual ~AI() = default;
};