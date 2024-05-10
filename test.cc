#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include "lock_free_queue.h"


struct Timer
{
    std::chrono::high_resolution_clock::time_point _start, _finish;

    void start()      { _start = std::chrono::high_resolution_clock::now(); }
    void stop()       { _finish = std::chrono::high_resolution_clock::now(); }
    auto elapsed_ms() { return std::chrono::duration_cast<std::chrono::microseconds>(_finish - _start).count(); }
};


template <typename TQueue, typename TValue>
auto test(auto capacity, auto objects_count, auto produce_time, auto consume_time)
{
    TQueue queue(capacity);

    std::vector<TValue> src(objects_count), dst(objects_count);
    std::ranges::for_each(src, [](auto & v){ v = rand(); });

    Timer timer;
    timer.start();

    std::thread producer([&src, &queue, produce_time]
    {
        size_t index = 0;
        while (index != src.size())
        {
            if (queue.try_push(src[index]))
            {
                ++index;
                std::this_thread::sleep_for(std::chrono::microseconds(produce_time));
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&dst, &queue, consume_time]
    {
        size_t index = 0;
        while (index != dst.size())
        {
            if (queue.try_pop(dst[index]))
            {
                ++index;
                std::this_thread::sleep_for(std::chrono::microseconds(consume_time));
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
    timer.stop();

    if (!std::ranges::equal(src, dst))
    {
        std::cout << "Content is not equal!" << std::endl;
    }

    return timer.elapsed_ms();
}


int main(int argc, char* argv[])
{
    size_t queue_capacity = std::atoll(argv[1]);
    size_t objects_count = std::atoll(argv[2]);
    size_t produce_time = std::atoll(argv[3]);
    size_t consume_time = std::atoll(argv[4]);
    std::cout << "queue_capacity: " << queue_capacity
              << ", objects_count: " << objects_count
              << ", produce_time, mcs: " << produce_time
              << ", consume_time, mcs: " << consume_time
              << std::endl;

    constexpr size_t REPEATS = 51;
    size_t repeats = REPEATS;
    size_t lfq_signed = 0, lfq_unsigned = 0, lfq_blind = 0;
    while (repeats--)
    {
        lfq_signed   += test<LockFreeQueue<uint64_t>, uint64_t>(queue_capacity, objects_count, produce_time, consume_time);
        lfq_unsigned += test<LockFreeQueueUnsignedIndex<uint64_t>, uint64_t>(queue_capacity, objects_count, produce_time, consume_time);
        lfq_blind    += test<LockFreeQueueBlind<uint64_t>, uint64_t>(queue_capacity, objects_count, produce_time, consume_time);

        if (repeats == REPEATS - 1)
        {
            lfq_signed = 0;
            lfq_unsigned = 0;
            lfq_blind = 0;
        }
    }

    std::cout << "LockFreeQueue signed index type measures: " << lfq_signed / (REPEATS - 1) << " mcs" << std::endl;
    std::cout << "LockFreeQueue unsigned index type measures: " << lfq_unsigned / (REPEATS - 1) << " mcs" << std::endl;
    std::cout << "LockFreeQueue blind measures: " << lfq_blind / (REPEATS - 1) << " mcs" << std::endl << std::endl;
}