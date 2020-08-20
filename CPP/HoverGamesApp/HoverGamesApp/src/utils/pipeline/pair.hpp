
#ifndef MODULE_PAIR
#define MODULE_PAIR

#include "module.hpp"

#include <vector>
#include <thread>
#include <atomic>
#include <type_traits>

namespace Scarecrow
{
    namespace Utils
    {
        template <typename T_IN, typename T_MID, typename T_OUT>
        class Pair: public Module<T_IN, T_OUT>
        {
        public:

            /// <summary>
            ///  A Pair is exectuded from "left" to "right" where T_IN is the input of left, T_MID is the ouput of left and the input of right and T_OUT is the output of the pair AND right
            /// </summary>
            /// <param name="left"></param>
            /// <param name="right"></param>

            Pair(Module<T_IN, T_MID>* left, Module<T_MID, T_OUT>* right):_left(left), _right(right){};
            ~Pair() 
            {
                // Handle memory
            };

        public:

            // NO INPUT, NO OUTPUT
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<IN_T>::value && std::is_void<OUT_T>::value && !std::is_void<MID_T>::value, void>::type
            execute()
            {
                auto out = _left->execute();
                _right->execute(out);
            };
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<IN_T>::value&& std::is_void<OUT_T>::value && std::is_void<MID_T>::value, void>::type
            execute()
            {
                _left->execute();
                _right->execute();
            };

            // INPUT, NO OUTPUT
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<OUT_T>::value && !std::is_void<MID_T>::value, void>::type
            execute(IN_T&& input)
            {
                auto out = _left->execute(std::forward<IN_T>(input));
                _right->execute(out);
            };
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<OUT_T>::value && std::is_void<MID_T>::value, void>::type
            execute(IN_T&& input)
            {
                _left->execute(std::forward<IN_T>(input));
                _right->execute();
            };

            // NO INPUT, OUTPUT
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<IN_T>::value && !std::is_void<MID_T>::value, OUT_T>::type
            execute()
            {
                auto out = _left->execute();
                return _right->execute(out);
            };
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<std::is_void<IN_T>::value && std::is_void<MID_T>::value, OUT_T>::type
            execute()
            {
                _left->execute();
                return _right->execute();
            };

            // INPUT, OUPUT
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<!std::is_void<IN_T>::value && !std::is_void<OUT_T>::value && !std::is_void<MID_T>::value, OUT_T>::type
            execute(IN_T&& input)
            {
                auto out = _left->execute(std::forward<IN_T>(input));
                return _right->execute(out);
            };
            template <typename IN_T = T_IN, typename MID_T = T_MID, typename OUT_T = T_OUT>
            typename std::enable_if<!std::is_void<IN_T>::value && !std::is_void<OUT_T>::value && std::is_void<MID_T>::value, OUT_T>::type
            execute(IN_T&& input)
            {
                _left->execute(std::forward<IN_T>(input));
                return _right->execute();
            };

        private:
            Module<T_IN, T_MID>     *_left;
            Module<T_MID, T_OUT>    *_right;
        };
    }
}

#endif
