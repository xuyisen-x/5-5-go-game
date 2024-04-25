#include <filesystem>
#include <fstream>
#include <thread>

#include <unistd.h>
#include <signal.h>

#include "constant.h"
#include "GoGame/GoGame.h"
#include "AI/MCTSAI.h"

/**
 * @brief return the number of stones on the board.
 * @param board: the current board, a 1D array of size board_size * board_size. 0 for empty, 1 for black, 2 for white.
 * 
 * @return int: the number of stones on the board.
 */
int getNStones(int* board);
/**
 * @brief judge one process exists or not
 * @param pid_t: the pid of the process
 * 
 * @return bool: the process exists or not
*/
bool isProcessExist(pid_t pid);

/**
 * @brief remove the useless file in the directory
 * @param dirStr: the directory path
 */
void removeUselessFile(const char* dirStr);

/**
 * @brief get the number of moves
 * @param logPathObj: the path of the log file
 * @param board: the current board, a 1D array of size board_size * board_size. 0 for empty, 1 for black, 2 for white.
 * @param nowPiece: the piece that should be placed. 1 for black, 2 for white.
 * 
 * @return int: the number of moves
*/
int getNMove(std::filesystem::path logPathObj, int* board, int nowPiece);

/**
 * @brief return the directory of the path
 * @param path: the path
 * 
 * @return std::filesystem::path: the directory of the path
*/
std::filesystem::path returnDirectory(std::filesystem::path path);

/**
 * @brief write the number of moves to the log file
 * @param logPathObj: the path of the log file
 * @param nMove: the number of moves
*/
void writeNMove(std::filesystem::path logPathObj, int nMove);

/**
 * @brief This function will be called by the get_input function in the MyPlayer class in Python.
 * 
 * @param board: the current board, a 1D array of size board_size * board_size. 0 for empty, 1 for black, 2 for white.
 * @param previousBoard: the previous board, a 1D array of size board_size * board_size. 0 for empty, 1 for black, 2 for white.
 * @param nowPiece: the piece that should be placed. 1 for black, 2 for white.
 * @param onnxPath: the path of onnx file
 * @param timeLimit: the time limit for the AI to think, in seconds.
 * @param logPath: the path of the log file
 * 
 * @return int: the index of cross point in the board to place the stone. i * board_size + j, where i is the row index and j is the column index. -1 for pass.
*/
extern "C" int get_input(int* board, int* previousBoard, int nowPiece, const char * onnxPath, int timeLimit, const char* logPath) {
    
    using namespace std::filesystem;

    // get the log file path
    // get the parent process id, the parent process identify one game
    std::string fileName = std::to_string(getppid()) + ".txt";
    path logPathObj = returnDirectory(path(logPath)) / fileName;

    int nMove = getNMove(logPathObj, board, nowPiece);

    // Create a GoGame object.
    GoGame game(board, previousBoard, nowPiece, nMove);

    // write the nMove to the log file
    writeNMove(logPathObj, game.getNMove());

    // Show the board and liberties, for debugging.
    // game.showBoard();
    // game.showLiberties();

    // Remove useless file, avoiding some game ended early
    std::thread removeThread(removeUselessFile, logPath);
    removeThread.detach();

    // Create a AI object
    TimeLimitMCTSAI ai = TimeLimitMCTSAI(onnxPath, 2, timeLimit);
    int move = boardPairToInt(ai.move(game));
    
    return move;
}

int getNStones(int* board)
{
    int n = 0;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
        if (board[i] != 0) n++;
    return n;
}

bool isProcessExist(pid_t pid)
{
    if (kill(pid, 0) == 0)
    {
        return true;
    }
    return false;
}

void removeUselessFile(const char* dirStr)
{
    using namespace std::filesystem;
    path dirPah;
    if(is_directory(dirStr)) {
        dirPah = path(dirStr);
    } else {
        dirPah = path(dirStr).parent_path();
    }

    try{
        for (const auto& entry : directory_iterator(dirPah))
        {
            std::string fileName = entry.path().filename().string();
            if (fileName.find(".txt") != std::string::npos)
            {
                pid_t parentID = std::stoi(fileName.substr(0, fileName.find(".txt")));
                if (!isProcessExist(parentID))
                {
                    remove(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

int getNMove(std::filesystem::path logPathObj, int* board, int nowPiece)
{
    int nMove = -1; // the number of moves

    // if the num of stones less than 2, the nMove depends on nowPiece
    if (getNStones(board) <= 1)
    {
        if (nowPiece == 1) nMove = 0;
        else nMove = 1;
    }
    // if the log file is exist, read the log file to get the nMove
    else {
        std::ifstream logFile(logPathObj);
        if (logFile.is_open())
        {
            logFile >> nMove;
            logFile.close();
            nMove += 2;
        }
    }

    return nMove;
}

std::filesystem::path returnDirectory(std::filesystem::path path)
{
    if (is_directory(path))
    {
        return path;
    }
    else
    {
        return path.parent_path();
    }
}

void writeNMove(std::filesystem::path logPathObj, int nMove)
{
    if (nMove != -1 && nMove < 22) {
        std::ofstream logFile(logPathObj);
        if (logFile.is_open())
        {
            logFile << nMove;
            logFile.close();
        }
    }
    else
    {
        remove(logPathObj);
    }
}