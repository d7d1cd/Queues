#pragma once

#include <atomic>
#include <vector>


template <typename T>
class LockFreeQueue
{
public:
    using ValueType   = T;
    using StorageType = std::vector<ValueType>;
    using IndexType   = int64_t;

    LockFreeQueue(IndexType capacity) : _mask(capacity - 1)
    {
        if (capacity < 2)
            throw std::runtime_error("Capacity too small");

        if (capacity & _mask)
            throw std::runtime_error("Capacity is not power of 2");

        _storage.resize(capacity);
        _writer.avail = capacity;
    }


    bool try_push(T data)
    {
        auto cur = _writer.cur.load(std::memory_order_relaxed);

        if (_writer.avail == 0) [[unlikely]]
        {
            _writer.avail = _storage.size() - (cur - _reader.cur.load(std::memory_order_acquire));
            if (_writer.avail == 0) [[unlikely]]
                return false;
        }

        _storage[cur & _mask] = std::move(data);
        _writer.cur.store(cur + 1, std::memory_order_release);
        --_writer.avail;
        return true;
    }


    bool try_pop(T & data)
    {
        auto cur = _reader.cur.load(std::memory_order_relaxed);

        if (_reader.avail == 0) [[unlikely]]
        {
            _reader.avail = _writer.cur.load(std::memory_order_acquire) - cur;
            if (_reader.avail == 0) [[unlikely]]
                return false;
        }

        data = std::move(_storage[cur & _mask]);
        _reader.cur.store(cur + 1, std::memory_order_release);
        --_reader.avail;
        return true;
    }


private:
    struct Position
    {
        std::atomic<IndexType> cur = 0;
        IndexType              avail = 0;
    };

    alignas(64) StorageType     _storage;
    alignas(64) const IndexType _mask;
    alignas(64) Position        _writer;
    alignas(64) Position        _reader;
};



template <typename T>
class LockFreeQueueUnsignedIndex
{
public:
    using StorageType = std::vector<T>;
    using IndexType   = uint64_t;

    LockFreeQueueUnsignedIndex(IndexType capacity) : _mask(capacity - 1)
    {
        if (capacity < 2)
            throw std::runtime_error("Capacity too small");

        if (capacity & _mask)
            throw std::runtime_error("Capacity is not power of 2");

        _storage.resize(capacity);
        _writer.avail = capacity;
    }


    bool try_push(T data)
    {
        auto cur = _writer.cur.load(std::memory_order_relaxed);

        if (_writer.avail == 0) [[unlikely]]
        {
            _writer.avail = _storage.size() - (cur - _reader.cur.load(std::memory_order_acquire));
            if (_writer.avail == 0) [[unlikely]]
                return false;
        }

        _storage[cur & _mask] = std::move(data);
        _writer.cur.store(cur + 1, std::memory_order_release);
        --_writer.avail;
        return true;
    }


    bool try_pop(T & data)
    {
        auto cur = _reader.cur.load(std::memory_order_relaxed);

        if (_reader.avail == 0) [[unlikely]]
        {
            _reader.avail = _writer.cur.load(std::memory_order_acquire) - cur;
            if (_reader.avail == 0) [[unlikely]]
                return false;
        }

        data = std::move(_storage[cur & _mask]);
        _reader.cur.store(cur + 1, std::memory_order_release);
        --_reader.avail;
        return true;
    }


private:
    struct Position
    {
        std::atomic<IndexType> cur = 0;
        IndexType              avail = 0;
    };

    alignas(64) StorageType     _storage;
    alignas(64) const IndexType _mask;
    alignas(64) Position        _writer;
    alignas(64) Position        _reader;
};



template <typename T>
class LockFreeQueueBlind
{
public:
    using StorageType = std::vector<T>;
    using IndexType   = uint64_t;

    LockFreeQueueBlind(IndexType capacity) : _mask(capacity - 1)
    {
        if (capacity < 2)
            throw std::runtime_error("Capacity too small");

        if (capacity & _mask)
            throw std::runtime_error("Capacity is not power of 2");

        _storage.resize(capacity);
    }


    bool try_push(T data)
    {
        auto cur = _writer.load(std::memory_order_relaxed);
        if ((cur - _reader.load(std::memory_order_acquire) == _storage.size())) [[unlikely]]
        {
            return false;
        }

        _storage[cur & _mask] = std::move(data);
        _writer.store(cur + 1, std::memory_order_release);
        return true;
    }


    bool try_pop(T & data)
    {
        auto cur = _reader.load(std::memory_order_relaxed);
        if (_writer.load(std::memory_order_acquire) == cur) [[unlikely]]
        {
            return false;
        }

        data = std::move(_storage[cur & _mask]);
        _reader.store(cur + 1, std::memory_order_release);
        return true;
    }


private:
    alignas(64) StorageType            _storage;
    alignas(64) const IndexType        _mask;
    alignas(64) std::atomic<IndexType> _writer;
    alignas(64) std::atomic<IndexType> _reader;
};



class PointerQueue : public LockFreeQueue<void *>
{
    using Base = LockFreeQueue<void *>;

public:
    using Base::Base;

    template <typename P>
    requires(std::is_pointer_v<P>)
    bool try_push(P pointer)
    {
        return Base::try_push(pointer);
    }

    template <typename P>
    requires(std::is_pointer_v<P>)
    bool try_pop(P & pointer)
    {
        Base::ValueType p;
        if (Base::try_pop(p)) [[likely]]
        {
            pointer = static_cast<P>(p);
            return true;
        }

        return false;
    }
};
