#include<iostream>
#include<memory>
#include<iomanip>

#include "GoGame/GoGame.h"
#include "AI/RandomAI.h"
#include "AI/MCTSAI.h"

int main(int argc, char *argv[])
{
    std::unique_ptr<AI> ai;

    GoGame game = GoGame();
    Player aiPlayer;

    while (true)
    {
        std::cout << "选择执黑(1)或执白(2):" << std::endl;
        int i;
        std::cin >> i;
        if (i == 1)
        {
            aiPlayer = Player::White;
            break;
        }
        if (i == 2)
        {
            aiPlayer = Player::Black;
            break;
        }
    }

    while (true)
    {
        std::cout << "ai思考时长：单位（秒）" << std::endl;
        int i;
        std::cin >> i;
        if (i > 0)
        {
            ai = std::make_unique<TimeLimitMCTSAI>("/home/xuyisen/project/Go_game/KataGoLike/python/model9.onnx", 2, i);
            break;
        }
    }
     

    while(!game.isGameOver())
    {
        game.showBoard();
        std::cout << "Steps:" << game.getNMove() << std::endl;
        if(game.getNowPiece() == aiPlayer)
        {
            std::cout << "Ai Move" << std::endl;
            auto [i,j] = ai->move(game);
            game.move(i,j);
        }
        else
        {
            std::cout << "You Move" << std::endl;
            bool flag = false;
            while (!flag)
            {
                int i, j;
                std::cin >> i >> j;
                flag = game.move(i,j);
            }
        }
    }

    game.showBoard();
    if(game.judgeWinner() == aiPlayer)
    {
        std::cout << "AI Win" << std::endl;
    }
    else
    {
        std::cout << "You Win" << std::endl;
    }

    return 0;
}