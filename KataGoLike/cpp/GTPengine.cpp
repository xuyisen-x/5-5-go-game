#include<iostream>
#include<memory>
#include<iomanip>

#include "GoGame/GoGame.h"
#include "AI/RandomAI.h"
#include "AI/MCTSAI.h"

void removeControlCharsExceptHTandLF(std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\t' || c == '\n' || (c >= 32 && c <= 126)) {
            // 保留水平制表符（HT），换行符（LF），以及所有可打印字符
            result += c;
        }
    }
    str = result;
}

void removeAfterCharacter(std::string& str, char character) {
    size_t pos = str.find(character);
    if (pos != std::string::npos) str.erase(pos);
}

std::vector<std::string> splitString(const std::string& str) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, ' ')) {
        tokens.push_back(token);
    }
    return tokens;
}

void successOutput(int id, const std::string& output)
{
    std::cout << "=";
    if (id != -1)
        std::cout << id;
    std::cout << " " << output << "\n\n";
}

void errorOutput(int id, const std::string& output)
{
    std::cout << "?";
    if (id != -1)
        std::cout << id;
    std::cout << " " << output << "\n\n";
}

int main(int argc, char *argv[])
{
    const std::set<std::string> know_commands = 
    {
        "protocol_version",
        "name",
        "version",
        "known_command"
        "list_commands",
        "quit",
        "boardsize",
        "clear_board",
        "komi",
        "play",
        "genmove"
    };

    GoGame game = GoGame();
    auto ai = TimeLimitMCTSAI("/home/xuyisen/project/Go_game/KataGoLike/python/model9.onnx", 2, 10);
    float black_wr = 0.0;

    while (true)
    {
        std::string input;
        std::getline(std::cin, input);
        removeControlCharsExceptHTandLF(input);   // 删除所有出现的 CR 和其他控制字符（HT 和 LF 除外）。
        removeAfterCharacter(input, '#');         // 对于带有井号 (#) 的每一行，删除该字符后面（包括该字符）的所有文本。
        std::replace(input.begin(), input.end(), '\t', ' ');  // 将所有出现的 HT 转换为 SPACE。
        removeAfterCharacter(input, '\n');        // 末尾的换行符也可以丢弃

        auto sInput = splitString(input);
        std::string command = "";
        int id = -1;
        std::vector<std::string> args;

        if (sInput.size() == 0) continue;

        if (std::isdigit(sInput[0][0]))
        {
            id = std::stoi(sInput[0]);
            if (sInput.size() == 1) continue;
            command = sInput[1];
            args = std::vector<std::string>(sInput.begin() + 2, sInput.end());
        }
        else
        {
            command = sInput[0];
            args = std::vector<std::string>(sInput.begin() + 1, sInput.end());
        }

        std::cout << "Received: " << command << std::endl;
        
        if (command == "protocol_version")
        {
            successOutput(id, "2");
        }
        else if (command == "name")
        {
            successOutput(id, "5*5special");
        }
        else if (command == "version")
        {
            successOutput(id, "version 1.0");
        }
        else if (command == "known_command")
        {
            if (args.size() < 1)
            {
                errorOutput(id, "known_command requires exactly 1 argument");
                continue;
            }
            if (know_commands.find(args[0]) != know_commands.end())
            {
                successOutput(id, "true");
            }
            else
            {
                successOutput(id, "false");
            }
        }
        else if (command == "list_commands")
        {
            std::string output = "";
            for (const auto& c : know_commands)
            {
                output += c + " ";
            }
            successOutput(id, output);
        }
        else if (command == "quit")
        {
            successOutput(id, "");
            break;
        }
        else if (command == "boardsize")
        {
            if (args.size() < 1)
            {
                errorOutput(id, "boardsize requires exactly 1 argument");
                continue;
            }
            int size = std::stoi(args[0]);
            if (size != 5)
            {
                errorOutput(id, "unacceptable size");
                continue;
            }
            successOutput(id, "");
        }
        else if (command == "clear_board")
        {
            game = GoGame();
            successOutput(id, "");
        }
        else if (command == "komi")
        {
            if (args.size() < 1)
            {
                errorOutput(id, "komi requires exactly 1 argument");
                continue;
            }
            float komi = std::stof(args[0]);
            if (komi != 2.5)
            {
                errorOutput(id, "unacceptable komi");
                continue;
            }
            successOutput(id, "");
        }
        else if (command == "play")
        {
            if (args.size() < 2)
            {
                errorOutput(id, "play requires exactly 2 arguments");
                continue;
            }
            if (args[0] == "black" || args[0] == "B")
            {
                if (game.getNowPiece() != Player::Black)
                {
                    errorOutput(id, "illegal move");
                    continue;
                }
            }
            else if (args[0] == "white" || args[0] == "W")
            {
                if (game.getNowPiece() != Player::White)
                {
                    errorOutput(id, "illegal move");
                    continue;
                }
            }
            else
            {
                errorOutput(id, "unacceptable color");
                continue;
            }

            bool result;

            if(args[1] == "pass")
            {
                result = game.move(-1, -1);
            }
            else
            {
                int x = args[1][0] - 'A';
                int y = args[1][1] - '1';
                result = game.move(x,y);
            }

            if (result)
            {
                successOutput(id, "");
            }
            else
            {
                errorOutput(id, "illegal move");
            }
        }
        else if (command == "genmove")
        {
            if (args.size() < 1)
            {
                errorOutput(id, "genmove requires exactly 1 argument");
                continue;
            }
            std::string color = args[0];
            
            if (color == "black" || color == "B")
            {
                if (game.getNowPiece() != Player::Black)
                {
                    errorOutput(id, "illegal move");
                    continue;
                }
            }
            else if (color == "white" || color == "W")
            {
                if (game.getNowPiece() != Player::White)
                {
                    errorOutput(id, "illegal move");
                    continue;
                }
            }
            else
            {
                errorOutput(id, "unacceptable color");
                continue;
            }

            if (game.isGameOver())
            {
                successOutput(id, "pass");
                continue;
            }

            auto [i, j, bwr] = ai.evaMove(game);
            black_wr = bwr;
            game.move(i, j);

            if (i == -1)
            {
                successOutput(id, "pass");
            }
            else
            {
                successOutput(id, std::string(1, 'A' + i) + std::to_string(j + 1));
            }

            std::cout << "\nblack win rate:" << std::fixed << std::setprecision(2) << bwr << std::endl;
        }
        else if (command == "p-nmove")
        {
            successOutput(id, std::to_string(game.getNMove()));
        }
        else if (command == "p-winner")
        {
            if (game.judgeWinner() == Player::Black)
            {
                successOutput(id, "black");
            }
            else if (game.judgeWinner() == Player::White)
            {
                successOutput(id, "white");
            }
            else
            {
                successOutput(id, "draw");
            }
        }
        else if (command == "p-bwr")
        {
            successOutput(id, std::to_string(black_wr));
        }
        else
        {
            errorOutput(id, "unknown command");
        }
    }
}