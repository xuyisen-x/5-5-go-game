#include <algorithm>

#include "ONNXEngine.h"

ONNXEngine::ONNXEngine(const char *onnxModelPath, unsigned int threadNum)
{
    _env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING,"MyOnnxRuntimeModel");
    Ort::SessionOptions sessionOptions{};
    if (threadNum > 0)
        sessionOptions.SetIntraOpNumThreads(threadNum);
    if (threadNum == 1)
        sessionOptions.SetInterOpNumThreads(threadNum);

    //TODO: add more options

    _session = std::make_unique<Ort::Session>(*_env, onnxModelPath, sessionOptions);
    _memoryInfo = std::make_unique<Ort::MemoryInfo>(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
    _runOptions = std::make_unique<Ort::RunOptions>();
}

ONNXEngine::ONNXEngine(ONNXEngine&& other) noexcept
{
    _env = std::move(other._env);
    _session = std::move(other._session);
    _memoryInfo = std::move(other._memoryInfo);
    _runOptions = std::move(other._runOptions);
}

ONNXEngine& ONNXEngine::operator=(ONNXEngine&& other) noexcept
{
    if(this != &other) 
    {
        _env = std::move(other._env);
        _session = std::move(other._session);
        _memoryInfo = std::move(other._memoryInfo);
        _runOptions = std::move(other._runOptions);
    }
    return *this;
}

void ONNXEngine::inference(float *input, float *output, size_t batchSize)
{
    _inputSize[0] = batchSize;
    // Create input tensor, and copy input data to it
    Ort::Value inputTensor = 
        Ort::Value::CreateTensor<float>(*_memoryInfo, input, 
                                        batchSize*NUMBER_OF_INPUT_CHANNELS*BOARD_SIZE*BOARD_SIZE, 
                                        (int64_t*)_inputSize, 4);
    // Run inference
    auto result = _session->Run(*_runOptions, INPUT_NAMES, &inputTensor, 1, OUTPUT_NAMES, 1);
    // Copy output data to output arrays
    std::copy_n(result[0].GetTensorMutableData<float>(), batchSize*(BOARD_SIZE*BOARD_SIZE + 1), (float*)output);
}
