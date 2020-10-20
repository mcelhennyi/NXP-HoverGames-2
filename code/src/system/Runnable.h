//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_RUNNABLE_H
#define HOVERGAMES2_RUNNABLE_H


#include <thread>
#include <unistd.h>
#include <iostream>

#include "../utils/time.h"

class Runnable
{
public:
    Runnable(float runnerRate=1.0): _rate(runnerRate), _delayUs((1/_rate) * 1000000), _running(false), _thread(nullptr)
    {

    };
    ~Runnable()
    {
        if(_running)
        {
            stop();
        }
    };

public:
    void setup()
    {
        doSetup();
    };

    void run()
    {
        _running = true;

        // Setup the thread and start it
        _thread = new std::thread(&Runnable::internalRun, this);
    };

    void stop()
    {
        _running = false;
        _thread->join();
        delete _thread;
        _thread = nullptr;
        doStop();
    };

protected:
    void internalRun()
    {
        while(_running)
        {
            auto beforeTime = Utils::Time::microsNow();
            doRun();
            auto timeAfter = Utils::Time::microsNow();

            // Calculate sleep time
            auto timeTaken = (timeAfter - beforeTime); // seconds
            if(timeTaken < _delayUs)
            {
                usleep(_delayUs - timeTaken);
            }
            else
            {
                std::cout << "Thread took too long" << std::endl;
            }
        }
    }

    virtual void doSetup() {};
    virtual void doRun() {};
    virtual void doStop() {};

private:

    bool        _running;
    float       _rate;
    float       _delayUs;
    std::thread *_thread;
};



#endif //HOVERGAMES2_RUNNABLE_H
