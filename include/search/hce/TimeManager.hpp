#ifndef VENGINE_SEARCH_TIMEMANAGER_H
#define VENGINE_SEARCH_TIMEMANAGER_H
#include <algorithm>
#include <cmath>
#include "core/Constants.hpp"

namespace VEngine {
    struct TimeAllocation {
        int optimumMs;
        int maximumMs;
    };

    class TimeManager {
    public:
        static TimeAllocation calculateTime(int wtime, int btime, int winc, int binc, int sideToMove, int ply = 30, int movesToGo = 0) {
            double timeLeft = (sideToMove == White) ? (double)wtime : (double)btime;
            double increment = (sideToMove == White) ? (double)winc : (double)binc;

            if (timeLeft <= 0) return {10, 10}; 
            
            double pingOverhead = 500.0;
            double engineFloor = 50.0; 
            
            if (timeLeft <= pingOverhead + 20.0) return {20, 20}; 
            
            double safeTime = timeLeft - pingOverhead;

            double logTimeInSec = std::log10(std::max(1.0, safeTime) / 1000.0);
            
            double optConstant = std::min(0.0035 + 0.00041 * logTimeInSec, 0.0055);
            double maxConstant = std::max(3.8 + 3.2 * logTimeInSec, 3.1);

            double plyWeight = 1.0 + 0.42 * std::exp(-std::pow(ply - 42.0, 2) / 1100.0);

            double optScale, maxScale;

            if (movesToGo == 0) {
                optScale = std::min(0.012 + std::pow(ply + 2.5, 0.45) * optConstant, 0.18);
                maxScale = std::min(6.8, maxConstant + ply / 11.5);
            } else {
                optScale = (0.9 + ply / 105.0) / (double)movesToGo;
                maxScale = 2.2 + (movesToGo / 18.0);
            }

            double totalOpt = (safeTime * optScale * plyWeight) + (increment * 0.75);
            double totalMax = totalOpt * maxScale;

            double maxFraction = (increment > 0) ? 0.45 : 0.18;
            
            if (increment == 0 && safeTime < 12000) {
                maxFraction = 0.10; 
                totalOpt *= 0.8;    
            }

            int finalOpt = std::clamp((int)totalOpt, (int)engineFloor, (int)(safeTime * 0.12));
            int finalMax = std::clamp((int)totalMax, (int)engineFloor, (int)(safeTime * maxFraction));

            if (finalMax < finalOpt) finalMax = finalOpt;

            return {finalOpt, finalMax};
        }
    };
}

#endif // VENGINE_SEARCH_TIMEMANAGER_H