
#ifndef MODULE_PIPELINE
#define MODULE_PIPELINE

#include "module.hpp"

#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <cstdlib>


namespace Scarecrow
{
    namespace Utils
    {
        template <typename T_IN, typename T_MID, typename T_OUT>
        class ModulePipeline
        {
        public:
            ModulePipeline(Pair<T_IN, T_MID, T_OUT>* pair, float frequency): _pair(pair), _freq(frequency) {};
            ~ModulePipeline() 
            {
                // TODO: delete memory
            };

        public:
            bool runPipeline()
            {
                _running = true;
                _pipelineThread = new std::thread(&ModulePipeline::pipeline, this);
            };

            bool shutdownPipeline()
            {
                _running = false;
            };

        private:

            void pipeline()
            {
                // TODO: First setup the pipelines


                while(_running)
                {
                    // Take start time
                    auto timeStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                    //Execute pair
                    //_pair->execute()

                    // Calculate time to execute
                    auto timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


                    // Sleep to meet frequency req
                    float sleepTime = (1.0 / _freq) - (timeEnd - timeStart);
                    if (sleepTime > 0)
                    {
                        Sleep(sleepTime); // millis
                    }
                    else
                    {
                        std::cout << "Pipeline blew frame by " << -sleepTime << " ms!" << std::endl;
                    }
                    
                }

                // TODO: Shutdown the pipelines

            };

        private:
            std::thread                 *_pipelineThread;
            std::atomic_bool            _running;

            Pair<T_IN, T_MID, T_OUT>    *_pair;
            float                       _freq;
        };
    }
}

#endif
