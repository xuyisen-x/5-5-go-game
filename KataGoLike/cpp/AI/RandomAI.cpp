#include "RandomAI.h"

std::pair<int, int> RandomAI::move(const GoGame& game){
    auto possiblePlacements = game.getPossiblePlacements();
    if (possiblePlacements.size() == 0) {
        return {-1, -1};
    }
    else
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(0, possiblePlacements.size() - 1);
        return possiblePlacements[distribution(gen)];
    }
}