
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

            // NO INPUT, NO OUTPUT
            template <typename IN_T = T_IN, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<IN_T>::value && std::is_void<OUT_T>::value, void>::type
            execute() {};

            // INPUT, NO OUTPUT
            template <typename IN_T = T_IN, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<OUT_T>::value, void>::type
            execute(IN_T&& input) {};

            // NO INPUT, OUTPUT
            template <typename IN_T = T_IN, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<IN_T>::value, OUT_T>::type
            execute() {};

            // INPUT, OUPUT
            template <typename IN_T = T_IN, typename OUT_T = T_OUT>
            typename std::enable_if<!std::is_void<IN_T>::value && !std::is_void<OUT_T>::value, OUT_T>::type
            execute(IN_T&& input) {};
            

            // TODO: We need a virtual function here, but cannot due to templated function which was required due to the typename line enable if stuff.....so we need an alternative way to hide a function...
            // maybe I need just diff base classes based on input and output availability....ugh



            bool shutdown() {};
        };
    }
}

#endif
