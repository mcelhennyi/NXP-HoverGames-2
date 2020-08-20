
#ifndef MODULE_PIPELINE
#define MODULE_PIPELINE

#include "module.hpp";

#include <vector>
#include <thread>
#include <atomic>

namespace Scarecrow
{
    namespace Utils
    {
        class ModulePipeline
        {
        public:
            ModulePipeline() {};
            ~ModulePipeline() {};

        public:
            void addModule(Module * module)
            {
                _modules.emplace_back(module);
            };

            bool setupPipeline();

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
                // First setup the pipelines
                for(auto &mod: _modules)
                {
                    mod->setup();
                }


                while(_running)
                {
                    for(auto &mod: _modules)
                    {
                        mod->run();
                    }
                }

                // Shutdown the pipelines
                for(auto &mod: _modules)
                {
                    mod->shutdown();
                }

            };

        private:
            std::thread             *_pipelineThread;
            std::atomic_bool        _running;

            std::vector<Module*>    _modules;
        };
    }
}

#endif
