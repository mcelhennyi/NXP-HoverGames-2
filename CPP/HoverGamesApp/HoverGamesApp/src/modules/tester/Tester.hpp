#ifndef MODULE_TESTER
#define MODULE_TESTER

#include "utils/pipeline/module.hpp"

namespace Scarecrow
{
    namespace Modules
    {
        class Tester :public Utils::Module<std::string, std::string>
        {
        public:
            Tester(std::string output):Utils::Module<std::string, std::string>(),  _output(output) {};
            ~Tester() {};

        public:
            std::string execute(std::string&& input)
            {
                std::cout << "I GOT " << input << " sending " << _output << std::endl;
                return _output;
            }


        private:
            std::string _output;
        };
    }
}


#endif