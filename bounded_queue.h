#include <atomic>
#include <vector>


template <typename T>
class BoundedQueue
{
    using StorageType = std::vector<T>;
    using IndexType   = int64_t;

    struct Position
    {
        std::atomic<IndexType> cur = 0;
        IndexType              avail = 0;
    };

    alignas(64) StorageType     _storage;
    alignas(64) const IndexType _mask;          
    alignas(64) Position        _writer;
    alignas(64) Position        _reader;

public:
    BoundedQueue(IndexType capacity) : _mask(capacity - 1)
    {
        _storage.resize(capacity);
        _writer.avail = capacity;
    }

    bool try_push(T t)
    {
        auto cur = _writer.cur.load(std::memory_order_relaxed);
        
        if (_writer.avail == 0) [[unlikely]]
        {
            _writer.avail = _storage.size() - (cur - _reader.cur.load(std::memory_order_acquire));
            if (_writer.avail == 0) [[unlikely]]
                return false;
        }
        
        _storage[cur & _mask] = std::move(t);
        _writer.cur.store(cur + 1, std::memory_order_release);
        --_writer.avail;
        return true;      
    }

    bool try_pop(T & t)
    {
        auto cur = _reader.cur.load(std::memory_order_relaxed);

        if (_reader.avail == 0) [[unlikely]]
        {
            _reader.avail = _writer.cur.load(std::memory_order_acquire) - cur;
            if (_reader.avail == 0) [[unlikely]]
                return false;
        }

        t = std::move(_storage[cur & _mask]);
        _reader.cur.store(cur + 1, std::memory_order_release);
        --_reader.avail;
        return true;
    }
};
