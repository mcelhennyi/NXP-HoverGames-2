
#ifndef MODULE
#define MODULE

namespace Scarecrow
{
    namespace Utils
    {
        template<typename T_IN, typename T_OUT>
        class Module
        {
        public:
            Module() {};
            ~Module() {};

        public:
            bool setup() {};

            virtual T_OUT execute(T_IN&& input)=0;

            bool shutdown() {};
        };
    }
}

#endif
