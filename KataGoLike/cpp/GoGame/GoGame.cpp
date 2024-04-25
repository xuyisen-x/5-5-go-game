# include "GoGame.h"

int boardPairToInt(Point p)
{
    if (p == std::make_pair(-1, -1)) return -1;
    return p.first * BOARD_SIZE + p.second;
}

Point boardIntToPair(int i)
{
    if (i == -1) return std::make_pair(-1, -1);
    return {i / BOARD_SIZE, i % BOARD_SIZE};
}

void GoGame::PieceGroupMap::addStone(int i, int j)
{
    auto pos = std::make_pair(i, j);
    // parent is itself
    _parent[pos]    = std::make_pair(i, j);
    // children of a stone include itself
    _children[pos]  = {std::make_pair(i, j)};
    // liberties is empty
    _liberties[pos] = {};
}

void GoGame::PieceGroupMap::addLiberty(int i, int j, int x, int y)
{
    auto parent = _parent.at({i, j});
    _liberties[parent].insert({x, y});
}

void GoGame::PieceGroupMap::removeLiberty(int i, int j, int x, int y)
{
    auto parent = _parent.at({i, j});
    _liberties[parent].erase({x, y});
}

void GoGame::PieceGroupMap::merge(int i1, int j1, int i2, int j2)
{
    // find
    auto parent1 = _parent.at({i1, j1});
    auto parent2 = _parent.at({i2, j2});

    // if these two in the same group return
    if (parent1 == parent2) return;

    // remove each other in liberties
    _liberties[parent1].erase({i2, j2});
    _liberties[parent2].erase({i1, j1});

    // make sure the group of parent1 is larger than the group of parent2
    if (_children[parent1].size() < _children[parent2].size())
    {
        std::swap(parent1, parent2);
    }

    // all stones of group2 become stones of group1
    for (auto child : _children[parent2])
    {
        _parent[child] = parent1;
        _children[parent1].insert(child);
    }
    _children.erase(parent2);

    // all liberties of group2 become liberties of group1
    for (auto liberty : _liberties[parent2])
    {
        _liberties[parent1].insert(liberty);
    }
    _liberties.erase(parent2);
}

int GoGame::PieceGroupMap::getLibertyNum(int i, int j) const
{
    if (!_parent.contains({i, j})) return -1;
    auto parent = _parent.at({i, j});
    return _liberties.at(parent).size();
}

int GoGame::PieceGroupMap::getChildrenNum(int i, int j) const
{
    if (!_parent.contains({i, j})) return -1;
    auto parent = _parent.at({i, j});
    return _children.at(parent).size();
}

const std::set<Point>& GoGame::PieceGroupMap::getChildren(int i, int j) const
{
    if (!_parent.contains({i, j})) 
        throw std::out_of_range("The position is not in the board");
    auto parent = _parent.at({i, j});
    return _children.at(parent);
}

void GoGame::PieceGroupMap::removeGroup(int i, int j)
{
    auto parent = _parent.at({i, j});
    for (auto child : _children[parent])
    {
        _parent.erase(child);
    }
    _children.erase(parent);
    _liberties.erase(parent);
}


GoGame::GoGame()
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            _board[i][j]         = Stone::Empty;
            _previousBoard[i][j] = Stone::Empty;
        }
    }
}

GoGame::GoGame(int* board, int* previousBoard, int nowPiece, int nMove):GoGame()
{
    int nBlack = 0; int nWhite = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            placeStone(i, j, static_cast<Stone>(board[i * BOARD_SIZE + j]));
            _previousBoard[i][j] = static_cast<Stone>(previousBoard[i * BOARD_SIZE + j]);

            if (_board[i][j] == Stone::Black) nBlack += 1;
            else if (_board[i][j] == Stone::White) nWhite += 1;
        }
    }
    _nowPiece = static_cast<Player>(nowPiece);

    if (nMove == -1)
    {
        // guess the _nMove
        if (_nowPiece == Player::Black)
        {
            _nMove = 2 * std::max(nBlack, nWhite);
        }
        else
        {
            _nMove = 2 * std::max(nBlack - 1, nWhite) + 1;
        }
    }
    else
    {
        // do not guess the _nMove
        _nMove = nMove;
    }
    
}

std::vector<Point> GoGame::getNeighbors(int i, int j) const
{
    std::vector<Point> neighbors{};
    if (i - 1 >= 0)         neighbors.push_back(std::make_pair(i - 1, j));
    if (i + 1 < BOARD_SIZE) neighbors.push_back(std::make_pair(i + 1, j));
    if (j - 1 >= 0)         neighbors.push_back(std::make_pair(i, j - 1));
    if (j + 1 < BOARD_SIZE) neighbors.push_back(std::make_pair(i, j + 1));
    return neighbors;
}

bool GoGame::isLegal(int i, int j, Stone stone) const
{
    if (stone == Stone::Empty) return false;
    if (_board[i][j] != Stone::Empty) return false;
    if (i < 0 || i >= BOARD_SIZE || j < 0 || j >= BOARD_SIZE) return false;

    // check
    bool legal = false;
    for (auto [x, y] : getNeighbors(i, j))
    {
        // if there is an empty neighbor, it is legal
        if (_board[x][y] == Stone::Empty)
        {
            legal = true;
            break;
        }
        // if there is a stone of the same color with more than 1 liberty, it is legal
        else if (_board[x][y] == stone)
        {
            if (_pieceGroupMap.getLibertyNum(x, y) > 1)
            {
                legal = true;
                break;
            }
        }
        // if there is a stone of the opposite color with only 1 liberty, it is legal
        else
        {
            if (_pieceGroupMap.getLibertyNum(x, y) == 1)
            {
                legal = true;

                // check KO rule
                if (_pieceGroupMap.getChildrenNum(x, y) == 1)
                {
                    Board tempBoard = _board;
                    tempBoard[x][y] = Stone::Empty;
                    tempBoard[i][j] = stone;
                    if (tempBoard == _previousBoard)
                    {
                        return false;
                    }
                }
                break;
            }
        }
    }

    return legal;
}

std::vector<Point> GoGame::getPossiblePlacements() const
{
    std::vector<Point> possiblePlacements{};
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (isLegal(i, j, static_cast<Stone>(_nowPiece)))
            {
                possiblePlacements.push_back({i, j});
            }
        }
    }
    return possiblePlacements;
}

bool GoGame::placeStone(int i, int j, Stone stone)
{
    if (stone == Stone::Empty) return false;
    if (_board[i][j] != Stone::Empty) return false;
    _board[i][j] = stone;
    _pieceGroupMap.addStone(i, j);
    for (auto [x, y] : getNeighbors(i, j))
    {   
        // if there is a stone of the same color, merge them
        if (_board[x][y] == stone)
        {
            _pieceGroupMap.merge(i, j, x, y);
        }
        // if there is an empty neighbor, add liberty
        else if (_board[x][y] == Stone::Empty)
        {
            _pieceGroupMap.addLiberty(i, j, x, y);
        }
        // if there is a stone of the opposite color, remove it's liberty
        else
        {
            _pieceGroupMap.removeLiberty(x, y, i, j);
        }
    }
    return true;
}

void GoGame::removeNeighborGroups(int i, int j, Stone stone)
{
    Stone opponent = (stone == Stone::Black) ? Stone::White : Stone::Black;
    for (auto [x, y] : getNeighbors(i, j))
    {
        if (_board[x][y] == opponent)
        {
            if (_pieceGroupMap.getLibertyNum(x, y) == 0)
            {
                removeGroup(x, y);
            }
        }
    }
}

void GoGame::removeGroup(int i, int j)
{
    Stone opponent = (_board[i][j] == Stone::Black) ? Stone::White : Stone::Black;
    for (auto [x, y] : _pieceGroupMap.getChildren(i, j))
    {
        _board[x][y] = Stone::Empty;
        for (auto [x1, y1] : getNeighbors(x, y))
        {
            if (_board[x1][y1] == opponent)
            {
                _pieceGroupMap.addLiberty(x1, y1, x, y);
            }
        }
    }
    _pieceGroupMap.removeGroup(i, j);
}

bool GoGame::move(int i, int j)
{
    if (!(i == -1 && j ==-1) && !isLegal(i, j, static_cast<Stone>(_nowPiece))) return false;
    if (_isGameOver) return false;

    auto tempBoard = _previousBoard;
    _previousBoard = _board;
    // if the move is not pass, place the stone and remove the the opponent's group of stones if it has no liberty
    if (!(i == -1 && j ==-1))
    {
        placeStone(i, j, static_cast<Stone>(_nowPiece));
        removeNeighborGroups(i, j, static_cast<Stone>(_nowPiece));
    }
    _nMove += 1;
    _nowPiece = (_nowPiece == Player::Black) ? Player::White : Player::Black;

    // check if the game is over
    // Case 1: the number of moves reaches the maximum
    if (_nMove >= _maxMove) _isGameOver = true;
    // Case 2: two players all pass
    if (i == -1 && j == -1 && tempBoard == _board && _nMove >= 2) _isGameOver = true;

    return true;
}

bool GoGame::isGameOver() const
{
    return _isGameOver;
}

void GoGame::showBoard() const
{
    std::cout << "Board:" << std::endl;
    std::cout << "  0 1 2 3 4 "<< std::endl;
    std::cout << "  " << std::string(BOARD_SIZE * 2, '=') << std::endl;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        std::cout << i << "|";
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (_board[i][j] == Stone::Empty)
            {
                std::cout << "  ";
            }
            else if (_board[i][j] == Stone::Black)
            {
                std::cout << "X ";
            }
            else if (_board[i][j] == Stone::White)
            {
                std::cout << "O ";
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "  " << std::string(BOARD_SIZE * 2, '=') << std::endl;
}

void GoGame::showLiberties() const
{
    std::cout << "Liberties:" << std::endl;
    std::cout << std::string(BOARD_SIZE * 2, '=') << std::endl;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            auto lNum = _pieceGroupMap.getLibertyNum(i, j);
            if (lNum < 0)
                std::cout <<  "  ";
            else
                std::cout << _pieceGroupMap.getLibertyNum(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::string(BOARD_SIZE * 2, '=') << std::endl;
}

Player GoGame::judgeWinner(bool isPrint) const
{
    float nBlack = 0; float nWhite = KOMI;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (_board[i][j] == Stone::Black) nBlack += 1;
            else if (_board[i][j] == Stone::White) nWhite += 1;
        }
    }

    Player winner = (nBlack > nWhite) ? Player::Black : Player::White;
    if (isPrint)
    {
        std::cout << "Black: " << nBlack << std::endl;
        std::cout << "White: " << nWhite << std::endl;
        std::cout << "Winner: " << ((winner == Player::Black) ? "Black" : "White") << std::endl;
    }
    return winner;
}

Player GoGame::getNowPiece() const
{
    return _nowPiece;
}

Stone GoGame::getStone(int i, int j) const
{
    return _board[i][j];
}

int GoGame::getLibertyNum(int i, int j) const
{
    return _pieceGroupMap.getLibertyNum(i, j);
}

int GoGame::getNMove() const
{
    return _nMove;
}