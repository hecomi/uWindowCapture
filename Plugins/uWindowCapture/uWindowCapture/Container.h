#pragma once

#include <functional>
#include <map>
#include <mutex>


template <class Key, class Value>
class Map
{
public:
    Map() = default;
    ~Map() = default;

    void Add(const Key& key, const Value& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.emplace(key, value);
    }

    void Remove(const Key& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.erase(key);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
    }

    const Value& Find(const Key& key) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = map_.find(key);
        return it != map_.end() ? it->second : null_;
    }

    const Value& operator[](const Key& key) const
    {
        return Find(key);
    }

    const Value& FindIf(const std::function<bool(const Key& key, const Value& value)>& pred) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = std::find_if(
            map_.begin(),
            map_.end(),
            [pred](const auto& pair) 
            { 
                return pred(pair.first, pair.second); 
            });
        return it != map_.end() ? it->second : null_;
    }

    void ForEach(const std::function<void(const Key& key, Value& value)>& func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : map_)
        {
            func(pair.first, pair.second);
        }
    }

    void RemoveIf(const std::function<bool(const Key& key, const Value& value)>& pred)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = map_.begin(); it != map_.end();)
        {
            if (pred(it->first, it->second))
            {
                map_.erase(it++);
            }
            else
            {
                it++;
            }
        }
    }

private:
    std::map<Key, Value> map_;
    mutable std::mutex mutex_;
    const Value null_ = nullptr;
};