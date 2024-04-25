#pragma once

#include <atomic>

#include "../GoGame/GoGame.h"

class ExhaustiveTree
{
private:
    class Node
    {
    friend ExhaustiveTree;
    private:
        GoGame _state;
        std::pair<int, int> _move;
        Player _winner;
        std::vector<Node> _children{};
        static std::atomic<bool> finished;
        static std::atomic<int> count;
        static const int MAX_COUNT = 500000;
    public:
        Node() = default;
        Node(GoGame state);
        Node(GoGame state, std::pair<int, int> _move);
        ~Node() = default;
    };

    GoGame _state;
    Node   root;
    
public:
    ExhaustiveTree(GoGame state);
    std::optional<std::pair<int, int>> getMustWinMove();
    void stop();
    ~ExhaustiveTree() = default;
};

