#include "ExhaustiveTree.h"

std::atomic<bool> ExhaustiveTree::Node::finished = true;
std::atomic<int> ExhaustiveTree::Node::count = 0;

ExhaustiveTree::Node::Node(GoGame state)
{
    if (finished) return;
    if (count++ > MAX_COUNT)
    {
        finished = true;
        return;
    }

    _state = state;
    _move = std::make_pair(-1, -1);

    if (_state.isGameOver())
    {
        _winner = _state.judgeWinner();
        return;
    }

    auto moves = _state.getPossiblePlacements();
    _children.clear();
    _children.reserve(moves.size() + 1);

    for (int i = 0; i < moves.size() + 1; i++)
    {
        if (i == 0)
        {
            _children.emplace_back(_state, std::make_pair(-1, -1));
        }
        else
        {
            _children.emplace_back(_state, moves[i - 1]);
        }
        // 任意方式能赢，则当前节点能赢
        if (_children[i]._winner == _state.getNowPiece())
        {
            _winner = _state.getNowPiece();
            return;
        }
    }

    // 如果所有方式都不能赢，则当前节点不能赢
    _winner = _state.getNowPiece() == Player::Black ? Player::White : Player::Black;
}

ExhaustiveTree::Node::Node(GoGame state, std::pair<int, int> move)
{
    if (finished) return;
    if (count++ > MAX_COUNT)
    {
        finished = true;
        return;
    }

    _state = state;
    _move = move;
    _state.move(_move.first, _move.second);

    if (_state.isGameOver())
    {
        _winner = _state.judgeWinner();
        return;
    }

    auto moves = _state.getPossiblePlacements();
    _children.clear();
    _children.reserve(moves.size() + 1);

    for (int i = 0; i < moves.size() + 1; i++)
    {
        if (i == 0)
        {
            _children.emplace_back(_state, std::make_pair(-1, -1));
        }
        else
        {
            _children.emplace_back(_state, moves[i - 1]);
        }
        // 任意方式能赢，则当前节点能赢
        if (_children[i]._winner == _state.getNowPiece())
        {
            _winner = _state.getNowPiece();
            return;
        }
    }

    // 如果所有方式都不能赢，则当前节点不能赢
    _winner = _state.getNowPiece() == Player::Black ? Player::White : Player::Black;
}

ExhaustiveTree::ExhaustiveTree(GoGame state)
{
    _state = state;
}

std::optional<std::pair<int, int>> ExhaustiveTree::getMustWinMove()
{
    if (_state.getNMove() < 14)
    {
        return std::nullopt;
    }

    Node::finished = false;
    Node::count = 0;

    root = Node(_state);

    if(root._winner == _state.getNowPiece())
    {
        for (auto child : root._children)
        {
            if (child._winner == _state.getNowPiece())
            {
                return std::make_optional(child._move);
            }
                
        }
    }
    return std::nullopt;
}

void ExhaustiveTree::stop()
{
    Node::finished = true;
}