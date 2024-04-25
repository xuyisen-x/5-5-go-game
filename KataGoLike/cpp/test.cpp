#include<iostream>
#include<memory>
#include<iomanip>

#include "GoGame/GoGame.h"
#include "AI/RandomAI.h"
#include "AI/MCTSAI.h"

int main(int argc, char *argv[])
{
    std::unique_ptr<AI> ai1 = std::make_unique<TimeLimitMCTSAI>("/home/xuyisen/project/Go_game/KataGoLike/python/model9.onnx", 2, 10);
    std::unique_ptr<AI> ai2 = std::make_unique<TimeLimitMCTSAI>("/home/xuyisen/project/Go_game/KataGoLike/python/model9.onnx", 2, 10);

    int ai1BlackWin = 0; int ai2BlackWin = 0;
    int ai1WhiteWin = 0; int ai2WhiteWin = 0;

    int simulationTimes = 30;

    for (int i = 0; i < simulationTimes; i++)
    {
        std::cout << i << std::endl;
        GoGame game = GoGame();
        game.move(2,2);
        game.move(3,2);
        game.move(1,1);
        while (!game.isGameOver())
        {
            game.showBoard();
            if (game.getNowPiece() == Player::Black)
            {
                if (i % 2 == 0)
                {
                    auto [i, j] = ai1->move(game);
                    if (!game.move(i, j)) return -1;
                }
                else
                {
                    auto [i, j] = ai2->move(game);
                    if (!game.move(i, j)) return -1;
                }
            }
            else
            {
                if (i % 2 == 0)
                {
                    auto [i, j] = ai2->move(game);
                    if (!game.move(i, j)) return -1;
                }
                else
                {
                    auto [i, j] = ai1->move(game);
                    if (!game.move(i, j)) return -1;
                }
            }

            if (game.isGameOver())
            {
                if (game.judgeWinner() == Player::Black)
                {
                    if (i % 2 == 0) ai1BlackWin++;
                    else ai2BlackWin++;
                }
                else
                {
                    if (i % 2 == 0) ai2WhiteWin++;
                    else ai1WhiteWin++;
                }
            }
        }

        std::cout << "AI1 Black Win: " << ai1BlackWin << std::endl;
        std::cout << "AI1 White Win: " << ai1WhiteWin << std::endl;
        std::cout << "AI2 Black Win: " << ai2BlackWin << std::endl;
        std::cout << "AI2 White Win: " << ai2WhiteWin << std::endl;
        std::cout << "AI1 Win Rate :"  << std::fixed << std::setprecision(2) << (float) (ai1BlackWin + ai1WhiteWin) / (i + 1) << std::endl;
        std::cout << "AI2 Win Rate :"  << std::fixed << std::setprecision(2) << (float) (ai2BlackWin + ai2WhiteWin) / (i + 1) << std::endl;
    }

    return 0;
}