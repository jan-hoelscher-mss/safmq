// randQTest.cpp : Defines the entry point for the console application.
//

#include "randpq.h"

int main()
{
    ra_priority_queue<int>  qu;
    for (int i = 3; i > 0; --i)
    {
        qu.enqueue(i);
    }
    qu.dequeue();
    return 0;
}