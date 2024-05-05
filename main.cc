#include "bounded_queue.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <algorithm>


void one_thread()
{
    constexpr std::size_t SIZE = 1048576;
    std::vector<int> src(SIZE), dst(SIZE);

    std::for_each(src.begin(), src.end(), [](auto & e){ e = rand(); });

    BoundedQueue<int> queue(SIZE); 

    std::for_each(src.begin(), src.end(), [&queue](const auto & e)
    { 
        assert(queue.try_push(e));
    });

    std::size_t i = 0;
    while (queue.try_pop(dst[i++]));

    assert(i - 1 == SIZE);

    assert(std::equal(src.begin(), src.end(), dst.begin()));
}


void two_thread()
{
    constexpr std::size_t SIZE = 1'048'576 * 2 * 2;
    std::vector<int> src(SIZE), dst(SIZE);

    std::for_each(src.begin(), src.end(), [](auto & e){ e = rand(); });

    BoundedQueue<int> queue(SIZE / 256); 

    std::thread producer([&src, &queue]
    {
        std::size_t i = 0;
        while (i != SIZE)
        {
            if (queue.try_push(src[i]))
            {
                ++i;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&dst, &queue, SIZE]
    {
        std::size_t i = 0;
        while (i != SIZE)
        {
            if (queue.try_pop(dst[i]))
            {
                ++i;
                std::this_thread::sleep_for(std::chrono::milliseconds(0));
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    assert(std::equal(src.begin(), src.end(), dst.begin()));
}


void two_thread2()
{
    std::cout << "std::numeric_limits<std::size_t>::max() = " << std::numeric_limits<std::size_t>::max() << std::endl;
    BoundedQueue<std::size_t> queue(1'048'576); 

    std::thread producer([&queue]
    {
        std::size_t i = 0;
        while (i < std::numeric_limits<std::size_t>::max())
        {
            if (queue.try_push(i))
            {
                ++i;
                // if (!(i & 134217727)) std::cout << "producer: i = " << i << std::endl;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&queue]
    {
        std::size_t i = 0;
        std::size_t val = 0;
        while (i < std::numeric_limits<std::size_t>::max())
        {
            if (queue.try_pop(val))
            {
                assert(i == val);
                ++i;
                // if (!(i & 134217727)) std::cout << "consumer: i = " << i << std::endl;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();
}


int main()
{
    // one_thread();
    two_thread2();
}