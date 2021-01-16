//
// Created by user on 1/15/21.
//

#ifndef HOVERGAMES2_THREADLOOP_H
#define HOVERGAMES2_THREADLOOP_H

#include <utils/thread/runnable/Runnable.h>
#include <functional>

namespace Utils
{
    namespace Thread
    {
        class ThreadLoop: public Runnable
        {
        public:
            ThreadLoop(std::function<void()> loopedThread, float rate): Runnable(rate), _func(std::move(loopedThread)) {};
            ~ThreadLoop() {};

        protected:
            void doSetup() override
            {

            }

            void doRun() override
            {
                _func();
            }

            void doStop() override
            {

            }

        private:
            std::function<void()> _func;
        };
    }
}

#endif //HOVERGAMES2_THREADLOOP_H
