//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_TIME_H
#define HOVERGAMES2_TIME_H

#include <chrono>

namespace Utils
{
    namespace Time
    {
        unsigned long microsNow()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }
    }
}

#endif //HOVERGAMES2_TIME_H
