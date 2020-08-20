
#ifndef MODULE_PAIR
#define MODULE_PAIR

#include "module.hpp"

#include <vector>
#include <thread>
#include <atomic>

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
            ~Pair() {};

        public:
            T_OUT execute(T_IN&& input)
            {
                return _right->execute(_left->execute(std::forward<T_IN>(input)));
            };

        private:
            Module<T_IN, T_MID>     *_left;
            Module<T_MID, T_OUT>    *_right;
        };
    }
}

#endif
