#include <memory>
#include <iostream>
#include <iomanip>
#include <thread>
#include <filesystem>
#include <string>
#include <mutex>
#include <atomic>
#include <random>

#include <H5Cpp.h>

#include "GoGame/GoGame.h"
#include "AI/MCTSAI.h"

const char* const ONNX_PATH = "/home/xuyisen/project/Go_game/KataGoLike/python/model9_1.onnx";

static std::mutex writeMutex{};
static H5::H5File file;
static H5::DataSpace inputSpace;
static H5::DataSpace outputSpace;
static H5::DataSet inputSet;
static H5::DataSet outputSet;
static std::atomic<int> count = 0;
constexpr int THREAD_NUM = 16;
constexpr int TOTALSTEPS = 125000;

void showOutputArray(const OutputArray& output)
{
    std::cout << "OutputArray:" << std::endl;
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            std::cout << std::fixed << std::setprecision(2) << output[boardPairToInt(std::make_pair(i,j))] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "pass:" << std::fixed << std::setprecision(2) << output[BOARD_SIZE * BOARD_SIZE] << std::endl;
}

void showInputArray(const InputArray& input)
{
    std::cout << "InputArray:" << std::endl;
    for(int i = 0; i < NUMBER_OF_INPUT_CHANNELS; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            for (int k = 0; k < BOARD_SIZE; k++)
            {
                std::cout << input[i][j][k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "=================" << std::endl;
    }
}

void selfPlayUnit(int index)
{

    // create dataset
    hsize_t inputMemDims[3]  = {5, 5, 5};
    hsize_t outputMemDims[1] = {26};
    hsize_t countInput[4]    = {1, 5, 5, 5};
    hsize_t countOutput[2]   = {1,26};
    hsize_t startInput[4]    = {0, 0, 0, 0};
    hsize_t startOutput[2]   = {0, 0};
    H5::DataSpace inputMemSpace(3, inputMemDims);
    H5::DataSpace outputMemSpace(1, outputMemDims);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    GoGame game = GoGame();
    MCTSAI ai = MCTSAI(ONNX_PATH, 800, 1, false);

    while (count < TOTALSTEPS)
    {
        if (distribution(gen) <= (1.0f / 20.0f))
        {
            auto [move, input, output] = ai.recordedMove(game);
            // write input, output to hdf5 file
            writeMutex.lock();
            if (count >= TOTALSTEPS)
            {
                writeMutex.unlock();
                break;
            }
            else
            {
                startInput[0] = count;
                startOutput[0] = count;
                inputSpace.selectHyperslab(H5S_SELECT_SET, countInput, startInput);
                outputSpace.selectHyperslab(H5S_SELECT_SET, countOutput, startOutput);
                inputSet.write(input.data(), H5::PredType::NATIVE_FLOAT, inputMemSpace, inputSpace);
                outputSet.write(output.data(), H5::PredType::NATIVE_FLOAT, outputMemSpace, outputSpace);
                count++;
                if(count % 100 == 99) std::cout << "step " << count + 1 << std::endl;
                writeMutex.unlock();
            }
            game.move(move.first, move.second);
        }
        else
        {
            auto move = ai.fastMove(game);
            game.move(move.first, move.second);
        }
        
        if (game.isGameOver()) game = GoGame();
    }
    std::cout << "Thread " << index << " finished" << std::endl;
}

int main(int argc, char *argv[])
{
    // HDF5 file to save the training data
    std::string fileName = "trainingData.hdf5";
    std::filesystem::path filePath = std::filesystem::path(HDF5_PATH) / fileName;
    // Create file
    file = H5::H5File(H5std_string(filePath.string()), H5F_ACC_TRUNC);

    // create dataset
    hsize_t inputSize[4]  = {TOTALSTEPS, 5, 5, 5};
    hsize_t outputSize[2] = {TOTALSTEPS, 26};
    inputSpace = H5::DataSpace(4, inputSize);
    outputSpace = H5::DataSpace(2, outputSize);
    inputSet = file.createDataSet("input", H5::PredType::NATIVE_FLOAT, inputSpace);
    outputSet = file.createDataSet("output", H5::PredType::NATIVE_FLOAT, outputSpace);
    count = 0;

    std::thread th[THREAD_NUM];
	for (int i = 0; i < THREAD_NUM; i++)
		th[i] = std::thread(selfPlayUnit, i);
	
    for (int i = 0; i < THREAD_NUM; i++)
		th[i].join();

    // release resources
    inputSet.close();
    outputSet.close();
    inputSpace.close();
    outputSet.close();
    file.close();
	
    return 0;
}