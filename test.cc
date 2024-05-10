#include <iostream>
#include <cassert>
#include "lock_free_queue.h"


int main()
{
    PointerQueue queue(2);

    int i = 42;
    assert(queue.try_push(&i));

    double d = 42.42;
    assert(queue.try_push(&d));

    int * ip = nullptr;
    assert(queue.try_pop(ip));
    assert(*ip == i);

    double * dp = nullptr;
    assert(queue.try_pop(dp));
    assert(*dp == d);
}
