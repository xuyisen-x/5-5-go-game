#pragma once

#include <onnxruntime_cxx_api.h>

#include "NeuralNetworkInferenceEngine.hpp"

/**
 * @brief A class that represents a TensorRT model for the AI.
 * 
 * This class inherits from the base Model class and provides functionality to use onnxruntime to inference.
 */
class ONNXEngine : public NeuralNetworkInferenceEngine{
public:
    /**
     * @brief Constructs a ONNXEngine object with the specified ONNX model path.
     * 
     * @param onnxModelPath The path to the ONNX model file.
     */
    explicit ONNXEngine(const char *onnxModelPath, unsigned int threadNum = DEFAULT_NUM_OF_INFERENCE_THREAD);
    ONNXEngine() = delete;
    ONNXEngine(ONNXEngine&& other) noexcept;
    ONNXEngine& operator=(ONNXEngine&& other) noexcept;
    ONNXEngine(const ONNXEngine&) = delete;
    ONNXEngine& operator=(const ONNXEngine&) = delete;

    /**
     * @brief Performs inference using the ONNX model.
     * 
     * @param input The input data for inference.
     * @param output1 The first output data of the inference.
     * @param output2 The second output data of the inference.
     * @param batchSize The size of the batch for inference.
     */
    void inference(float* input, float* output, size_t batchSize) override;

private:
    std::unique_ptr<Ort::Env> _env;
    std::unique_ptr<Ort::Session> _session;
    std::unique_ptr<Ort::MemoryInfo> _memoryInfo;
    std::unique_ptr<Ort::RunOptions> _runOptions;
    int64_t _inputSize[4] = {0, NUMBER_OF_INPUT_CHANNELS, BOARD_SIZE, BOARD_SIZE};
};
