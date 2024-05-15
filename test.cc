#include <iostream>
#include <cassert>
#include "gcl/include/lock_free_buffer_queue.h"


int main()
{
    gcl::lock_free_buffer_queue<int> queue(1);

    auto r = queue.nonblocking_push(42);
    std::cout << static_cast<int>(r) << std::endl;

    r = queue.nonblocking_push(84);
    std::cout << static_cast<int>(r) << std::endl;

    int v = 0;
    r = queue.nonblocking_pop(v);
    std::cout << static_cast<int>(r) << std::endl;
    std::cout << v << std::endl;
}
