#pragma once

#include <array>
#include <vector>
#include <set>
#include <map>
#include <iostream>

#include "../constant.h"
#include "../utils/BoardMap.hpp"

// Define the Stone and Player enum classes.
enum class Stone
{
    Empty = 0,
    Black = 1,
    White = 2
};
enum class Player
{
    Black = 1,
    White = 2
};

// Define the Board and Point types.
typedef std::array<std::array<Stone, BOARD_SIZE>, BOARD_SIZE> Board;
typedef std::pair<int, int> Point;

/**
 * @brief Convert a point on the board into an integer index.
 * @param p: the pair of integers.
 * @return int: the integer index.
 */
int boardPairToInt(Point p);

/**
 * @brief Convert an integer index into a point on the board.
 * @param i: the integer index.
 * @return Point: the pair of integers.
 */
Point boardIntToPair(int i);

/**
 * @brief The GoGame class.
 * 
 * This class is used to save the state of the game and help with following tasks, including:
 * 1. get some features of the board, such as the neighbors of a point, the possible placements of the next stone, etc.
 * 2. judge the game state, such as whether a placement is legal, whether the game is over, etc.
 */
class GoGame
{
    private:
        /**
         * @brief The PieceGroupMap class.
         * 
         * This class is used to save the groups of stones which are connected to each other.
         * This class can help with following tasks, including:
         * 1. get stones which are in the same group of a specific stone.
         * 2. get the liberties of a specific group of stones.
         *
         * This class borrows the idea from Find-Union Set:
         * 1. Find: _parent[i][j] is the parent of the stone at (i, j).
         * 2. Union: merge function is used to merge two groups of stones.
         * https://en.wikipedia.org/wiki/Disjoint-set_data_structure
         */
        class PieceGroupMap
        {
            private:
                // The parent of each stone, {-1, -1} means the stone is not in any group.
                BoardMap<Point>                         _parent    = {};
                // The children of parents.
                BoardMap<std::set<std::pair<int, int>>> _children  = {};
                // The liberties of parents.
                BoardMap<std::set<std::pair<int, int>>> _liberties = {};
            public:
                PieceGroupMap()                                    = default;
                PieceGroupMap(const PieceGroupMap& map)            = default;
                PieceGroupMap(PieceGroupMap&& map)                 = default;
                PieceGroupMap& operator=(const PieceGroupMap& map) = default;
                PieceGroupMap& operator=(PieceGroupMap&& map)      = default;
                ~PieceGroupMap()                                   = default;

                /**
                 * @brief merge two groups of stones with specific stones.
                 * @param i1: the row index of the first stone.
                 * @param j1: the column index of the first stone.
                 * @param i2: the row index of the second stone.
                 * @param j2: the column index of the second stone.
                 */
                void merge(int i1, int j1, int i2, int j2);
                
                /**
                 * @brief add a stone to the map.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                 */
                void addStone(int i, int j);

                /**
                 * @brief add a liberty to a group of stones with a specific stone.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                 * @param x: the row index of the liberty.
                 * @param y: the column index of the liberty.
                 */
                void addLiberty(int i, int j, int x, int y);
                
                /**
                 * @brief remove a liberty from a group of stones with a specific stone.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                 * @param x: the row index of the liberty.
                 * @param y: the column index of the liberty.
                 */
                void removeLiberty(int i, int j, int x, int y);

                /**
                 * @brief get the number of liberties of a group of stones with a specific stone.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                 * @return int: the number of liberties.
                */
                int getLibertyNum(int i, int j) const;

                /**
                 * @brief get the number of stones in the same group of a specific stone.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                 * @return int: the number of stones.
                */
                int getChildrenNum(int i, int j) const;

                /**
                 * @brief get the stones of a group with a specific stone.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                */
                const std::set<std::pair<int, int>>& getChildren(int i, int j) const;

                /**
                 * @brief Remove a group of stones with a specific stone.
                 * @param i: the row index of the stone.
                 * @param j: the column index of the stone.
                 */
                void removeGroup(int i, int j);
        };

        // The map of groups of stones.
        PieceGroupMap _pieceGroupMap = {};
        // The board of the game.
        Board         _board;
        // The previous board of the game.
        Board         _previousBoard;
        // Now piece to move.
        Player        _nowPiece      = Player::Black;
        // The number of moves which have been made.
        int           _nMove         = 0;
        // is the game over.
        bool          _isGameOver    = false;
        // The maximum number of moves that can be made.
        static const int _maxMove = BOARD_SIZE * BOARD_SIZE - 1;

    public:
        GoGame();
        GoGame(const GoGame& game)            = default;
        GoGame(GoGame&& game)                 = default;
        GoGame& operator=(const GoGame& game) = default;
        GoGame& operator=(GoGame&& game)      = default;
        ~GoGame()                             = default;

        /**
         * @brief Construct a new GoGame object, copy the board, previous board and now piece given by python.
         * @param board: the current board, ctypes array of size board_size * board_size. 0 for empty, 1 for black, 2 for white.
         * @param previousBoard: the previous board, ctypes array of size board_size * board_size. 0 for empty, 1 for black, 2 for white.
         * @param nowPiece: the piece that should be placed. 1 for black, 2 for white.
         * @param nMove: the number of moves which have been made. -1 for not given.
         */
        GoGame(int* board, int* previousBoard, int nowPiece, int nMove = -1);

        /**
         * @brief Get the neighbors of a point.
         * @param i: the row index of the point.
         * @param j: the column index of the point.
         * @return std::vector<Point>: the neighbors of the point.
         */
        std::vector<Point> getNeighbors(int i, int j) const;

        /**
         * @brief Check whether a placement is legal.
         * @param i: the row index of the placement.
         * @param j: the column index of the placement.
         * @param stone: the stone to be placed.
         * @return bool: whether the placement is legal.
        */
        bool isLegal(int i, int j, Stone stone) const;

        /**
         * @brief Get the possible placements of the next stone.
         * @return std::vector<Point>: the possible placements.
         */
        std::vector<Point> getPossiblePlacements() const;

        /**
         * @brief place a stone on the board.
         * @return bool: whether the placement is successful.
         */
        bool placeStone(int i, int j, Stone stone);

        /**
         * @brief Remove the groups of stones which have no liberty.
         * @param i: the row index of the stone has been placed.
         * @param j: the column index of the stone has been placed.
         * @param stone: the stone has been placed.
         */
        void removeNeighborGroups(int i, int j, Stone stone);

        /**
         * @brief Remove a group of stones.
         * @param i: the row index of the stone.
         * @param j: the column index of the stone.
         */
        void removeGroup(int i, int j);

        /**
         * @brief Move a mvoe
         * @param i: the row index of the placement.
         * @param j: the column index of the placement.
         * @return bool: whether the move is successful.
         */
        bool move(int i, int j);

        /**
         * @brief Show the board.
         */
        void showBoard() const;

        /**
         * @brief Show the liberties of the groups of stones.
         */
        void showLiberties() const;

        /**
         * @brief Check whether the game is over.
         * @return bool: whether the game is over.
         */
        bool isGameOver() const;
        
        /**
         * @brief Judge the winner of the game.
         * @param isPrint: whether to print to the console.
         * @return Player: the winner of the game.
         */
        Player judgeWinner(bool isPrint = false) const;

        /**
         * @brief Now piece to move.
         * @return Player: the now piece to move.
         */
        Player getNowPiece() const;

        /**
         * @brief Get the stone at a specific point.
         * @param i: the row index of the point.
         * @param j: the column index of the point.
         * @return Stone: the stone at the point.
         */
        Stone getStone(int i, int j) const;

        /**
         * @brief Get the number of liberties of a group of stones with a specific stone.
         * @param i: the row index of the stone.
         * @param j: the column index of the stone.
         * @return int: the number of liberties.
        */
        int getLibertyNum(int i, int j) const;

        /**
         * @brief Get the number of moves which have been made.
         * @return int: the number of moves which have been made.
        */
        int getNMove() const;
};