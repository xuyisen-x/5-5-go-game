#pragma once

#include <cstdio>
#include <array>
#include <vector>

#include "../constant.h"



/**
 * @brief The NeuralNetworkInferenceEngine class represents an abstract base class for reasoners used in the system.
 *
 * This class provides an interface for performing inference on input data and obtaining output predictions.
 * It also defines constants related to the model's configuration.
 */
class NeuralNetworkInferenceEngine
{
public:
    /**
     * @brief Perform inference using the given input and store the results in the output arrays.
     *
     * This method is pure virtual and must be implemented by derived classes.
     *
     * @param input The input data for inference.
     * @param output The first output prediction, policy net output.
     * @param batchSize The size of the input batch.
     */
    virtual void inference(float *input, float *output, std::size_t batchSize) = 0;

    /**
     * @brief Perform inference using the given input and store the results in the output arrays.
     *
     * This method is a convenience wrapper around the pure virtual `inference` method.
     * It accepts input and output arrays with fixed sizes and delegates the actual inference to the `inference` method.
     *
     * @tparam batchSize The size of the input batch.
     * @tparam N The size of the first output prediction, N must be equal to batchSize.
     * @param input The input data for inference.
     * @param output The first output prediction, policy net output.
     */
    template <std::size_t batchSize, std::size_t N>
    void inference(const float (&input)[batchSize][NUMBER_OF_INPUT_CHANNELS][BOARD_SIZE][BOARD_SIZE], 
                   float (&output)[N][BOARD_SIZE * BOARD_SIZE + 1])
    {
        static_assert(batchSize == N, "batch size of input and outputs must be the same");
        inference((float *)&input, (float *)&output, batchSize);
    }
    
    /**
     * @brief Perform inference using the given input and store the results in the output arrays.
     * 
     * This method is a convenience wrapper around the pure virtual `inference` method.
     * It accepts input and output arrays with fixed sizes and delegates the actual inference to the `inference` method.
     * 
     * @tparam batchSize The batch size of the input data.
     * @tparam N The size of the first output array, N must be equal to batchSize.
     * @param input The input data for inference.
     * @param output The first output prediction, policy net output.
     */
    template <std::size_t batchSize, std::size_t N>
    void inference(const std::array<InputArray, batchSize>& input, 
                   std::array<OutputArray, N>& output)
    {
        static_assert(batchSize == N, "batch size of input and outputs must be the same");
        inference((float *)input.data(), (float *)output.data(), batchSize);
    }

    /**
     * @brief Perform inference using the given input and store the results in the output arrays.
     * 
     * This method is a convenience wrapper around the pure virtual `inference` method.
     * It accepts input and output arrays with fixed sizes and delegates the actual inference to the `inference` method.
     * 
     * @param input The input data for inference.
     * @param output The first output prediction, policy net output.
     */
    void inference(const std::vector<InputArray>& input, std::vector<OutputArray>& output)
    {
        inference((float *)input.data(), (float *)output.data(), input.size());
    }

    /**
     * @brief Perform inference using the given input and store the results in the output arrays.
     * 
     * This method is a convenience wrapper around the pure virtual `inference` method.
     * It accepts input and output arrays with fixed sizes and delegates the actual inference to the `inference` method.
     * 
     * @param input The input data for inference.
     * @param output The first output prediction, policy net output.
     */
    void inference(const InputArray& input, OutputArray& output)
    {
        inference((float *)input.data(), (float *)output.data(), 1);
    }

    /**
     * @brief Perform inference using the given input and store the results in the output arrays.
     * 
     * This method is a convenience wrapper around the pure virtual `inference` method.
     * It accepts input and output arrays with fixed sizes and delegates the actual inference to the `inference` method.
     * 
     * @param input The input data for inference.
     * @param output The first output prediction, policy net output.
     */
    void inference(const float (&input)[NUMBER_OF_INPUT_CHANNELS][BOARD_SIZE][BOARD_SIZE], 
                   float (&output)[BOARD_SIZE * BOARD_SIZE + 1])
    {
        inference((float *)&input, (float *)&output, 1);
    }

    virtual ~NeuralNetworkInferenceEngine() = default;
};