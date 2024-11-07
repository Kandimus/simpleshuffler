#include "main.h"
#include <string>
#include <vector>
#include <random>

// rand number [0, 0xffffffffffffffff]
std::string funcRand0(const std::vector<size_t>& args)
{
    return std::to_string(global::rand.get64(65536));
}

// rand number [0, limit)
std::string funcRand1(const std::vector<size_t>& args)
{
    size_t limit = args.size() == 1 ? args[0] : 65536;
    return limit ? std::to_string(global::rand.get64(limit)) : 0;
}

// rand number [0, limit)
std::string funcRand2(const std::vector<size_t>& args)
{
    size_t limitMin = args.size() == 2 ? args[0] : 0;
    size_t limitMax = args.size() == 2 ? args[1] : 65536;
    
    if (limitMin == limitMax)
    {
        return std::to_string(limitMin);
    }
    if (limitMin > limitMax)
    {
        size_t tmp = limitMin;
        limitMin = limitMax;
        limitMax = tmp;
    }

    return std::to_string(global::rand.get64(limitMax - limitMin) + limitMin);
}
