#pragma once

#include<optional>
#include<array>

#include"../constant.h"

template<class T>
class BoardMap
{
    private:
        std::array<std::array<std::optional<T>, BOARD_SIZE>,BOARD_SIZE> board;
    public:
        BoardMap()
        {
            for (auto& row : board)
            {
                for (auto& item : row)
                {
                    item.reset();
                }
            }
        }
        ~BoardMap() = default;
        BoardMap(const BoardMap&) = default;
        BoardMap& operator=(const BoardMap&) = default;
        BoardMap(BoardMap&&) = default;
        BoardMap& operator=(BoardMap&&) = default;

        bool contains(std::pair<int, int> pos) const
        {
            return board[pos.first][pos.second].has_value();
        }

        const T& at(std::pair<int, int> pos) const
        {
            if (!contains(pos))
            {
                throw std::out_of_range("The position is not in the board");
            }
            return board[pos.first][pos.second].value();
        }

        T& operator[](std::pair<int, int> pos)
        {
            if (!contains(pos))
            {
                insert(pos, T());
            }
            return board[pos.first][pos.second].value();
        }

        void erase(std::pair<int, int> pos)
        {
            board[pos.first][pos.second].reset();
        }

        void insert(std::pair<int, int> pos, T value)
        {
            board[pos.first][pos.second] = std::make_optional(value);
        }

};