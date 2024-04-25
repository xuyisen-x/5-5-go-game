#pragma once

#include <unordered_map>
#include <queue>

#include "../constant.h"

template<typename TKey, typename TValue, typename MyHash>
class FIFOCache
{
private:
    std::unordered_map<TKey, TValue, MyHash> _cache{};
    std::queue<TKey> _fifo{};
    size_t _size{0};
    size_t _maxSize = MAX_CACHE_SIZE;
public:
    FIFOCache(){}
    FIFOCache(size_t maxSize) : _maxSize(maxSize){}
    ~FIFOCache() = default;
    FIFOCache(const FIFOCache&) = default;
    FIFOCache& operator=(const FIFOCache&) = default;
    FIFOCache(FIFOCache&&) = default;
    FIFOCache& operator=(FIFOCache&&) = default;

    bool contains(const TKey& key)
    {
        return _cache.find(key) != _cache.end();
    }

    bool tryGet(const TKey& key, TValue& value)
    {
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            return true;
        }
        return false;
    }

    void put(const TKey& key, const TValue& value)
    {
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            it->second = value;
            return;
        }
        if (_size == _maxSize)
        {
            auto& keyToRemove = _fifo.front();
            _fifo.pop();
            _cache.erase(keyToRemove);
            _fifo.push(key);
            _cache[key] = value;
            return;
        }
        _fifo.push(key);
        _cache[key] = value;
        _size++;
    }
};