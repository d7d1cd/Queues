#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "bounded_queue.h"


struct Timer
{
    std::chrono::high_resolution_clock::time_point _start, _finish;

    void start()      { _start = std::chrono::high_resolution_clock::now(); }
    void stop()       { _finish = std::chrono::high_resolution_clock::now(); }
    auto elapsed_ms() { return std::chrono::duration_cast<std::chrono::microseconds>(_finish - _start).count(); }
};


template <typename TTransport>
auto test(auto transport_capacity, auto objects_count, auto produce_time, auto consume_time)
{
    TTransport transport(transport_capacity);

    std::vector<size_t> src(objects_count), dst(objects_count);
    std::for_each(src.begin(), src.end(), [](auto & v){ v = rand(); });

    Timer timer;
    timer.start();

    std::thread producer([&src, &transport, produce_time]
    {
        size_t index = 0;
        while (index != src.size())
        {
            if (transport.write_val(src[index]))
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

    std::thread consumer([&dst, &transport, consume_time]
    {
        size_t index = 0;
        while (index != dst.size())
        {
            if (transport.read_val(dst[index]))
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

    if (!std::equal(src.begin(), src.end(), dst.begin()))
    {
        std::cout << "Content is not equal!" << std::endl;
    }

    return timer.elapsed_ms();
}


int main(int argc, char* argv[])
{
    size_t transport_capacity = std::atoll(argv[1]);
    size_t objects_count = std::atoll(argv[2]);
    size_t produce_time = std::atoll(argv[3]);
    size_t consume_time = std::atoll(argv[4]);
    std::cout << "transport_capacity: " << transport_capacity
              << ", objects_count: " << objects_count
              << ", produce_time, mcs: " << produce_time
              << ", consume_time, mcs: " << consume_time
              << std::endl;

    constexpr size_t REPEATS = 51;
    size_t repeats = REPEATS;
    size_t pipe = 0, bqueue = 0;
    while (repeats--)
    {
        pipe   += test<common::PipeTransport>(transport_capacity * sizeof(size_t) + 1, objects_count, produce_time, consume_time);
        bqueue += test<BoundedQueue<size_t>>(transport_capacity, objects_count, produce_time, consume_time);

        if (repeats == REPEATS - 1)
        {
            pipe = 0;
            bqueue = 0;
        }
    }

    std::cout << "PipeTransport measures: " << pipe / (REPEATS - 1) << " mcs" << std::endl;
    std::cout << "BoundedQueue measures: " << bqueue / (REPEATS - 1) << " mcs" << std::endl << std::endl;
}