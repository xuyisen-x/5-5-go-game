#pragma once

#include <memory>
#include <future>

#include "AI.h"
#include "../Model/ONNXEngine.h"
#include "../utils/FIFOCache.hpp"

class InferenceEngine
{
    private:        
        std::unique_ptr<NeuralNetworkInferenceEngine> _engine;
    public:
        InferenceEngine(std::unique_ptr<NeuralNetworkInferenceEngine>&& engine);
        OutputArray inference(const GoGame& game);
};

class MCTNode
{
    friend class MCTSAI;
    friend class TimeLimitMCTSAI;
    private:
        MCTNode* _parent;
        std::vector<MCTNode*> _children;
        GoGame _state;
        std::pair<int, int> _action;

        float _P;
        int _visitTimes;
        int _blackWinTimes;
        int _whiteWinTimes;

        InferenceEngine* _engine;
        bool _isForceSelect;

        MCTNode* selectBestChild();
        std::pair<int, int> randomAction(const std::vector<std::pair<int, int>>& actions,
                                         const std::vector<float>& probs);
    
    public:
        // This constructor is used for root node
        MCTNode(const GoGame& game, InferenceEngine* engine, bool forceSelect);

        // This constructor is used for other nodes
        MCTNode(MCTNode* parent, std::pair<int, int> action, InferenceEngine* engine, float P);

        ~MCTNode();

        float getPUCT() const;

        bool isRoot() const;
        
        void expand();
        void select();
        void rollout();
        void setResult(int blackWinTimes, int whiteWinTimes);

};

class MCTSAI : public AI
{
    private:
        std::unique_ptr<InferenceEngine> _engine;
        int MTC_STEPS;
        bool _forceSelect;

    public:
        MCTSAI(const char* onnxPath, unsigned int steps = DEFAULT_ITERATION, unsigned int threadNum = DEFAULT_NUM_OF_INFERENCE_THREAD, bool forceSelect = false);
        void setMTCSteps(int steps);
        std::pair<int, int> move(const GoGame& game) override;
        std::pair<int, int> fastMove(const GoGame& game);
        std::tuple<std::pair<int,int>, InputArray, OutputArray> recordedMove (const GoGame& game);
};

// just for race
class TimeLimitMCTSAI : public AI
{
    private:
        std::unique_ptr<InferenceEngine> _engine;
        int _timeLimit;
        static const bool _forceSelect = true;
        static const int  _maxSteps    = 1000000;

    public:
        TimeLimitMCTSAI(const char* onnxPath, unsigned int threadNum, int timeLimit = 1);
        std::pair<int, int> move(const GoGame& game) override;
        void moveAsync(const GoGame& game, std::promise<std::pair<int, int>>& promise);
        std::tuple<int, int, float> evaMove(const GoGame& game); 
};