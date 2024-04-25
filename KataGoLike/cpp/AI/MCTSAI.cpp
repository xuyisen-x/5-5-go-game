#include <random>
#include <chrono>
#include <future>

#include"MCTSAI.h"
#include"ExhaustiveTree.h"

InputArray getFeatures(const GoGame& game)
{
    InputArray input  = {0};

    int thisPlayer = game.getNowPiece() == Player::Black ? 1 : -1;
    Stone thisStone = game.getNowPiece() == Player::Black ? Stone::Black : Stone::White;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            Stone stone = game.getStone(i, j);
            if (game.isLegal(i, j, thisStone))
            {
                input[4][i][j] = thisPlayer;
            }
            if (stone == Stone::Black)
            {
                input[0][i][j] = 1;
                input[2][i][j] = game.getLibertyNum(i, j);
            }
            else if (stone == Stone::White)
            {
                input[1][i][j] = 1;
                input[3][i][j] = game.getLibertyNum(i, j);
            }
        }
    }
    return input;
}

InferenceEngine::InferenceEngine(std::unique_ptr<NeuralNetworkInferenceEngine>&& engine)
{
    _engine = std::move(engine);
}

OutputArray InferenceEngine::inference(const GoGame& game)
{
    InputArray input  = getFeatures(game);
    OutputArray output = {0};

    _engine->inference(input, output);
    return output;
}

MCTNode* MCTNode::selectBestChild()
{
    if (_children.empty()) return nullptr;
    MCTNode* bestChild = nullptr;
    float bestPUCT = std::numeric_limits<float>::lowest();
    for (auto child : _children)
    {
        // FORCED SELECT
        if (_isForceSelect && child->_visitTimes < sqrt(FORCE_SELECT_K * _visitTimes))
            return child;
        float PUCT = child->getPUCT();
        if (PUCT > bestPUCT)
        {
            bestPUCT = PUCT;
            bestChild = child;
        }
    }
    return bestChild;
}

std::pair<int, int> MCTNode::randomAction(const std::vector<std::pair<int, int>>& actions,
                                          const std::vector<float>& probs)
{
    if (actions.size() != probs.size()) return {-1, -1};
    
    float sum = 0;
    std::vector<float> cumulativeWeights(probs.size());
    for (int i = 0; i < probs.size(); i++)
    {
        sum += probs[i];
        cumulativeWeights[i] = sum;
    }

    if (sum == 0) return {-1, -1};

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<> distrib(0.0, sum);
    double randomWeight = distrib(gen);

    auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(), randomWeight);
    int index = std::distance(cumulativeWeights.begin(), it);

    return actions[index];
}

MCTNode::MCTNode(const GoGame& game, InferenceEngine* engine, bool forceSelect)
{
    this->_parent = nullptr;
    this->_children = {};
    this->_state = game;
    this->_action = {-1, -1};
    this->_P = 0;
    this->_visitTimes = 0;
    this->_blackWinTimes = 0;
    this->_whiteWinTimes = 0;
    this->_engine = engine;
    this->_isForceSelect = forceSelect;
}

MCTNode::MCTNode(MCTNode* parent, std::pair<int, int> action, InferenceEngine* engine, float P)
{
    this->_parent = parent;
    this->_children = {};
    this->_state = parent->_state;
    this->_state.move(action.first, action.second);
    this->_action = action;
    this->_P = P;
    this->_visitTimes = 0;
    this->_blackWinTimes = 0;
    this->_whiteWinTimes = 0;
    this->_engine = engine;
    this->_isForceSelect = false;
}

MCTNode::~MCTNode()
{
    for (auto child : _children)
    {
        delete child;
    }
}

float MCTNode::getPUCT() const
{
    if (_parent == nullptr) return 0;

    if (_visitTimes == 0)
    {
        return 0 + C_PUCT * _P * sqrt(_parent->_visitTimes) / (1 + _visitTimes);
    }
    
    if (_state.getNowPiece() == Player::Black)
    {
        return (float)(_whiteWinTimes) / _visitTimes + C_PUCT * _P * sqrt(_parent->_visitTimes) / (1 + _visitTimes);
    }
    else
    {
        return (float)(_blackWinTimes) / _visitTimes + C_PUCT * _P * sqrt(_parent->_visitTimes) / (1 + _visitTimes);
    }
}

bool MCTNode::isRoot() const
{
    return _parent == nullptr;
}

void MCTNode::expand()
{
    if (!_children.empty()) return;
    
    OutputArray policy = {0};
    if constexpr (USE_NEURAL_NETWORK)
        policy = _engine->inference(_state);

    auto moves = _state.getPossiblePlacements();
    for (auto move : moves)
    {
        if constexpr (USE_NEURAL_NETWORK)
            _children.push_back(
                new MCTNode(this, move, _engine, policy[boardPairToInt(move)]));
        else
            _children.push_back(
                new MCTNode(this, move, _engine, 1.0f / (moves.size() + 1)));
    }
    // you can always pass
    if constexpr (USE_NEURAL_NETWORK)
        _children.push_back(
            new MCTNode(this, {-1, -1}, _engine, policy[BOARD_SIZE * BOARD_SIZE]));
    else
        _children.push_back(
            new MCTNode(this, {-1, -1}, _engine, 1.0f / (moves.size() + 1)));
}

void MCTNode::select()
{
    _visitTimes += 1;
    
    // if children is not empty, select the one child
    if (!_children.empty())
    {
        selectBestChild()->select();
        return;
    }

    // if game is over, backpropagate
    if (_state.isGameOver())
    {
        if (_state.judgeWinner() == Player::Black)
        {
            setResult(1, 0);
        }
        else
        {
            setResult(0, 1);
        }
        return;
    }

    // if is the first time to visit this node, rollout
    if (_visitTimes == 1)
    {
        rollout();
        return;
    }
    // else expand, and select the best child
    else
    {
        expand();
        selectBestChild()->select();
        return;
    }
}

void MCTNode::rollout()
{
    // copy 
    GoGame game = _state;
    
    while (!game.isGameOver())
    {
        OutputArray policy = {0};
        if constexpr (USE_NEURAL_NETWORK)
            policy = _engine->inference(game);

        // all legal moves
        auto legalMoves = game.getPossiblePlacements();

        std::vector<float> probs(legalMoves.size() + 1);
        for (int i = 0; i < legalMoves.size(); i++)
        {
            if constexpr (USE_NEURAL_NETWORK)
                probs[i] = policy[boardPairToInt(legalMoves[i])];
            else
                probs[i] = 1.0f / (legalMoves.size() + 1);
        }

        // you can always pass
        legalMoves.push_back({-1, -1});
        if constexpr (USE_NEURAL_NETWORK)
            probs[legalMoves.size() - 1] = policy[BOARD_SIZE * BOARD_SIZE];
        else
            probs[legalMoves.size() - 1] = 1.0f / legalMoves.size();

        auto [i, j] = randomAction(legalMoves, probs);
        game.move(i, j);
    }

    if (game.judgeWinner() == Player::Black)
    {
        setResult(1, 0);
    }
    else
    {
        setResult(0, 1);
    }
    
}

void MCTNode::setResult(int blackWinTimes, int whiteWinTimes)
{
    this->_blackWinTimes += blackWinTimes;
    this->_whiteWinTimes += whiteWinTimes;
    if (_parent != nullptr)
    {
        _parent->setResult(blackWinTimes, whiteWinTimes);
    }
}

MCTSAI::MCTSAI(const char* onnxPath, unsigned int steps, unsigned int threadNum, bool forceSelect)
{
    _engine = std::make_unique<InferenceEngine>(
        std::make_unique<ONNXEngine>(onnxPath, threadNum));
    MTC_STEPS = steps;
    _forceSelect = forceSelect;
}

void MCTSAI::setMTCSteps(int steps)
{
    MTC_STEPS = steps;
}

std::pair<int, int> MCTSAI::move(const GoGame& game)
{
    MCTNode root(game, _engine.get(), _forceSelect);
    for (int i = 0; i < MTC_STEPS; i++)
    {
        root.select();
    }
    
    int bestActionVistTimes = 0;
    MCTNode* bestChild = nullptr;

    for (auto child : root._children)
    {
        if (child->_visitTimes > bestActionVistTimes)
        {
            bestActionVistTimes = child->_visitTimes;
            bestChild = child;
        }
    }
    return bestChild->_action;
}

std::pair<int, int> MCTSAI::fastMove(const GoGame& game)
{
    MCTNode root(game, _engine.get(), _forceSelect);
    for (int i = 0; i < MTC_STEPS / 5; i++)
    {
        root.select();
    }
    
    int bestActionVistTimes = 0;
    MCTNode* bestChild = nullptr;

    for (auto child : root._children)
    {
        if (child->_visitTimes > bestActionVistTimes)
        {
            bestActionVistTimes = child->_visitTimes;
            bestChild = child;
        }
    }
    return bestChild->_action;
}

std::tuple<std::pair<int,int>, InputArray, OutputArray> MCTSAI::recordedMove (const GoGame& game)
{
    MCTNode root(game, _engine.get(), _forceSelect);
    for (int i = 0; i < MTC_STEPS; i++)
    {
        root.select();
    }

    int bestActionVistTimes = 0;
    MCTNode* bestChild = nullptr;

    OutputArray output = {0};
    int sum = 0;

    for (auto child : root._children)
    {
        sum += child->_visitTimes;
        int index = boardPairToInt(child->_action);
        index = index == -1 ? BOARD_SIZE * BOARD_SIZE : index;
        output[index] = child->_visitTimes;

        if (child->_visitTimes > bestActionVistTimes)
        {
            bestActionVistTimes = child->_visitTimes;
            bestChild = child;
        }
    }

    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE + 1; i++)
    {
        output[i] = (float) output[i] / sum;
    }

    return {bestChild->_action, getFeatures(game), output};
}

TimeLimitMCTSAI::TimeLimitMCTSAI(const char* onnxPath, unsigned int threadNum, int timeLimit)
{
    _engine = std::make_unique<InferenceEngine>(
        std::make_unique<ONNXEngine>(onnxPath, threadNum));
    _timeLimit = timeLimit;
}

std::pair<int, int> TimeLimitMCTSAI::move(const GoGame& game)
{
    std::promise<std::pair<int, int>> promise;
    auto future = promise.get_future();
    std::thread(&TimeLimitMCTSAI::moveAsync, this, game, std::ref(promise)).detach();
    return future.get();
}

void TimeLimitMCTSAI::moveAsync(const GoGame& game, std::promise<std::pair<int, int>>& promise)
{
    using namespace std::chrono;

    auto startTime     = steady_clock::now();
    auto fixedDuration = seconds(_timeLimit);

    auto eTree         = ExhaustiveTree(game);
    auto mustWinMove   = std::async(&ExhaustiveTree::getMustWinMove, &eTree);
    

    MCTNode root(game, _engine.get(), _forceSelect);
    for (int i = 0; i < _maxSteps; i++)
    {
        root.select();
        if (steady_clock::now() - startTime > fixedDuration)
        {
            break;
        }
    }

    // 判断是否有必胜走法
    eTree.stop(); 
    auto mustWinResult = mustWinMove.get();
    if(mustWinResult.has_value())
    {
        promise.set_value(mustWinResult.value());
        return;
    }
    
    int bestActionVistTimes = 0;
    MCTNode* bestChild = nullptr;

    for (auto child : root._children)
    {
        if (child->_visitTimes > bestActionVistTimes)
        {
            bestActionVistTimes = child->_visitTimes;
            bestChild = child;
        }
    }

    promise.set_value(bestChild->_action);
}

std::tuple<int, int, float> TimeLimitMCTSAI::evaMove(const GoGame& game)
{
    using namespace std::chrono;

    auto startTime     = steady_clock::now();
    auto fixedDuration = seconds(_timeLimit);

    auto eTree         = ExhaustiveTree(game);
    auto mustWinMove   = std::async(&ExhaustiveTree::getMustWinMove, &eTree);

    MCTNode root(game, _engine.get(), _forceSelect);
    for (int i = 0; i < _maxSteps; i++)
    {
        root.select();
        if (steady_clock::now() - startTime > fixedDuration)
        {
            break;
        }
    }

    // 判断是否有必胜走法
    eTree.stop(); 
    auto mustWinResult = mustWinMove.get();
    if(mustWinResult.has_value())
    {
        auto [x, y] = mustWinResult.value();
        float bwr;
        if(game.getNowPiece() == Player::Black)
        {
            bwr = 1;
        }
        else
        {
            bwr = 0;
        }
        return {x,y,bwr};
    }
    
    int bestActionVistTimes = 0;
    MCTNode* bestChild = nullptr;

    for (auto child : root._children)
    {
        if (child->_visitTimes > bestActionVistTimes)
        {
            bestActionVistTimes = child->_visitTimes;
            bestChild = child;
        }
    }

    float blackWinRate = (float) bestChild->_blackWinTimes / bestChild->_visitTimes;
    auto [x,y] = bestChild->_action;
    return {x, y, blackWinRate};
}

