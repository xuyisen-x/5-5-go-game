#pragma once

#include <array>
#include <cstddef> // size_t

constexpr int   BOARD_SIZE = 5;
constexpr float KOMI       = 2.5;
constexpr int   NUMBER_OF_INPUT_CHANNELS = 5;

typedef std::array<std::array<std::array<float,BOARD_SIZE>,BOARD_SIZE>,NUMBER_OF_INPUT_CHANNELS> InputArray;
typedef std::array<float,BOARD_SIZE * BOARD_SIZE + 1> OutputArray;

constexpr const char* const INPUT_NAMES[1]  = {"gameBoard"};
constexpr const char* const OUTPUT_NAMES[1] = {"policy"};

constexpr float C_PUCT = 1.1;

constexpr bool USE_NEURAL_NETWORK = true;

constexpr unsigned int DEFAULT_ITERATION = 400;

constexpr unsigned int DEFAULT_NUM_OF_INFERENCE_THREAD = 2;   // 0 for using all available threads

constexpr size_t MAX_CACHE_SIZE = 10000;

constexpr float FORCE_SELECT_K = 0.5;

constexpr const char* const HDF5_PATH = "/home/xuyisen/project/Go_game/KataGoLike/data/";