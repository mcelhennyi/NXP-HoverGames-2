//
// Created by user on 2/4/21.
//

#ifndef HOVERGAMES2_TICKER_H
#define HOVERGAMES2_TICKER_H

#include "util_time.h"
#include <string>
#include <iostream>

namespace Utils
{
    namespace Time
    {
        class Ticker
        {
        public:
            Ticker(float displayRate, std::string displayText, Ticker *previousTicker=nullptr):
                _displayText(displayText), _delayUs(1/displayRate * 1000000), _displayRate(displayRate), _lastDisplayTime(0),
                _previousTicker(previousTicker), _startTime(microsNow())
            {

            };
            ~Ticker(){};

            void tick(std::string displayText="")
            {
                if(!displayText.empty())
                    _displayText = displayText;

                ++_ticksSinceLastDisplay;
                auto timeNow = microsNow();

                if(timeNow - _lastDisplayTime > _delayUs) // && _ticksSinceLastDisplay > 0)
                {
                    if(_previousTicker)
                    {
                        std::cout << "[Elapsed Time: " << (float)((timeNow - _previousTicker->_lastTickTime) / 1000000.0f) << "] -- ";
                    }

                    std::cout << "[Display Time: " << (float)((timeNow - _startTime) / 1000000.0f) << "] -- " << _displayText <<
                    " -- (" << (_ticksSinceLastDisplay / (1.0f/_displayRate)) << " Hz)" << std::endl;
                    _lastDisplayTime = timeNow;
                    _ticksSinceLastDisplay = 0;
                }
            };

        private:
            Timestamp   _startTime;
            Timestamp   _lastTickTime;

            Timestamp   _lastDisplayTime;
            Timestamp   _delayUs;
            float       _displayRate;
            int         _ticksSinceLastDisplay;

            std::string _displayText;

            Ticker      *_previousTicker;

        };
    }
}

#endif //HOVERGAMES2_TICKER_H
