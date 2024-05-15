#pragma once

#include "gcl/include/lock_free_buffer_queue.h"


template <typename T>
struct GclQueue : gcl::lock_free_buffer_queue<T>
{
    using Base = gcl::lock_free_buffer_queue<T>;

    GclQueue(std::size_t capacity) : Base(capacity) {}

    bool try_push(T data)
    {
        return Base::nonblocking_push(data) == gcl::queue_op_status::success;
    }

    bool try_pop(T & data)
    {
        return Base::nonblocking_pop(data) == gcl::queue_op_status::success;
    }
};