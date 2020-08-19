
#ifndef MODULE
#define MODULE

namespace Scarecrow
{
    namespace Utils
    {
        class Module
        {
        public:
            Module();
            ~Module();

        public:
            bool setup();

            template<typename T_IN, typename T_OUT>
            T_OUT run(T_IN input);

            bool shutdown();
        };
    }
}

#endif
