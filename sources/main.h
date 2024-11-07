
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Random.h"

namespace fs = std::filesystem;

enum class KMState
{
    TokenBegin = 0,
    TokenEnd,
    OpenBraceBegin,
    OpenBraceEnd,
    NumberBegin,
    NumberEnd,
    SeparatorBegin,
    SeparatorEnd,
    CloseBrace,
    Semicolon,
    Success,
    Error,
};

typedef std::string(*fToken)(const std::vector<size_t>& args);

struct TokenInfo
{
    std::string name;
    size_t argsCount;
    fToken function;
};

struct FoundToken
{
    const TokenInfo* info;
    size_t begPos;
    size_t endPos;
    std::vector<size_t> args;
};

namespace global
{
extern std::string begMarker;
extern std::string endMarker;
extern std::string out;

extern std::string funcRand0;
extern std::string funcRand1;
extern std::string funcRand2;

extern std::vector<TokenInfo> tokens;
extern su::RandomXoshiro256PlusPlus rand;
}

std::string funcRand0(const std::vector<size_t>& args);
std::string funcRand1(const std::vector<size_t>& args);
std::string funcRand2(const std::vector<size_t>& args);

