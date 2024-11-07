
#include "main.h"

#include <algorithm>
#include <random>

#include <windows.h>

#include "commandline.h"
#include "stringex.h"

namespace arg
{
const su::CommandLineOption HELP = { "help", 'h' };
const su::CommandLineOption REPLACE = { "replace", 'r' };
const su::CommandLineOption NOBACKUP = { "nobackup", 'B' };
const su::CommandLineOption WEAK = { "weak", 'w' };
const su::CommandLineOption OUTPUT = { "out", 'o' };
const su::CommandLineOption THEAD = {"thread", 't'};
const su::CommandLineOption BEGMARKER = {"begmarker", 'm'};
const su::CommandLineOption ENDMARKER = {"endmarker", 'M'};
const su::CommandLineOption RAND0 = { "rand0", 0 };
const su::CommandLineOption RAND1 = { "rand1", 0 };
const su::CommandLineOption RAND2 = { "rand2", 0 };
}

namespace global
{
std::string begMarker = "//>>>BEGIN_SHUFFLING";
std::string endMarker = "//<<<END_SHUFFLING";
std::string out = "out";

std::string funcRand0 = "SHUFFLING_RAND0";
std::string funcRand1 = "SHUFFLING_RAND1";
std::string funcRand2 = "SHUFFLING_RAND2";

std::vector<TokenInfo> tokens;
su::RandomXoshiro256PlusPlus rand;
}

int doShuffle(const std::string& filename, bool replace, bool nobackup, bool weakrand);

int main(int argc, const char** argv)
{
    su::CommandLine cl;

    cl
        .addSwitch(arg::HELP, "Print this help")
        .addSwitch(arg::REPLACE, "Replace the source file")
        .addSwitch(arg::NOBACKUP, "Do not create backup of the source file")
        .addOption(arg::OUTPUT, global::out, "Output file")
        .addOption(arg::THEAD, "-1", "Do not implemented")
        .addOption(arg::BEGMARKER, global::begMarker, "The begin marker of the shuffle block")
        .addOption(arg::ENDMARKER, global::endMarker, "The end marker of the shuffle block")
        .addOption(arg::RAND0, global::funcRand0, "The name of SHUFFLING_RAND0() function")
        .addOption(arg::RAND1, global::funcRand1, "The name of SHUFFLING_RAND1(size_t) function")
        .addOption(arg::RAND2, global::funcRand2, "The name of SHUFFLING_RAND2(size_t, size_t) function")
        .parse(argc, argv);

    srand(time(0));

    global::begMarker = cl.getOption(arg::BEGMARKER);
    global::endMarker = cl.getOption(arg::ENDMARKER);
    global::out = cl.getOption(arg::OUTPUT);

    std::vector<std::string> files;

    for (int ii = 0; ii < cl.getCountArgument(); ++ii)
    {
        files.push_back(cl.getArgument(ii));
    }

    if (files.empty() || cl.isSet(arg::HELP))
    {
        
        printf("A simple shuffler designed to shuffle lines in source code files (C) Toadman\n");
        printf("version 1.6\n");
        printf("usage: simpleshuffler [options and switches] files\n");

        cl.printArguments();
        //printf("Options and switches:\n");
        //printf("\t--thread,-t=<count>\t\trun on <count> threads\n");
        //printf("\t--replace,-r\t\t\tcreate backup file and replace source file\n");
        //printf("\t--nobackup,-B\t\t\tdo not create backup file\n");
        //printf("\t--weak,-w\t\t\tusing a weak shuffle\n");
        //printf("\t--out=<extension>\t\tset a new output extension, default is '%s'.\n"
        //    "\t\t\t\t\tIgnoring if `--replase` switch is present.\n",
        //    global::out.c_str());
        //printf("\t--begmarker=<new begin marker>\tset a new begin marker, default is '%s'\n",
        //    global::begMarker.c_str());
        //printf("\t--endmarker=<new end marker>\tset a new end marker, default is '%s'\n",
        //    global::endMarker.c_str());

        return 1;
    }

    global::tokens.push_back(TokenInfo{ cl.getOption(arg::RAND0), 0, &funcRand0 });
    global::tokens.push_back(TokenInfo{ cl.getOption(arg::RAND1), 1, &funcRand1 });
    global::tokens.push_back(TokenInfo{ cl.getOption(arg::RAND2), 2, &funcRand2 });

    //TODO create thread_manager

    for (auto& item : files)
    {
        doShuffle(item, cl.isSet(arg::REPLACE.first), !cl.isSet(arg::NOBACKUP.first), cl.isSet(arg::WEAK.first));
    }

    return 0;
}

bool checkShufflingQuality(const std::vector<std::string>& source, const std::vector<std::string>& dest)
{
    if (source.size() != dest.size())
    {
        printf("Error: size of a sourse buffer and a destination buffer is not equal!\n");
        exit(1);
    }

    for (size_t ii = 0; ii < source.size(); ++ii)
    {
        if (source[ii] == dest[ii])
        {
            return false;
        }
    }

    return true;
}

void smartShuffle(std::vector<std::string>& lines, bool weakrand)
{
    if (lines.size() < 2)
    {
        return;
    }

#ifndef _WIN32
    std::random_device rd;
    std::mt19937 g(rd());
#endif

    auto oldLines = lines;

    size_t old_index = 0;
    size_t new_index = 0;
    size_t checkCount = 0;
    double dRand = double(lines.size() - 1) / double(RAND_MAX);

    while (true)
    {
        
#ifdef _WIN32
        new_index = size_t(rand() * dRand);
#else
        new_index = g() * (source.size() - 1) / RAND_MAX;
#endif

        if (new_index == old_index)
        {
            continue;
        }

        std::iter_swap(lines.begin() + old_index, lines.begin() + new_index);

        if (++old_index >= lines.size())
        {
            if (weakrand)
            {
                break;
            }

            if (checkShufflingQuality(oldLines, lines))
            {
                break;
            }

            checkCount = 0;
            old_index = 0;
        }
        else if (checkCount++ >= 3)
        {
            if (checkShufflingQuality(oldLines, lines))
            {
                break;
            }
            checkCount = 0;
        }
    }
}

void removeWhitespace(const std::string& text, size_t& pos)
{
    while ((text[pos] == ' ' || text[pos] == '\t' ||
        text[pos] == '\r' || text[pos] == '\n') && pos < text.size())
    {
        ++pos;
    }
}

const TokenInfo* getToken(std::string& text)
{
    for (const auto& token : global::tokens)
    {
        if (token.name == text)
        {
            return &token;
        }
    }

    return nullptr;
}

bool isFirstCharOfName(char ch)
{
    return std::isalpha(ch) || ch == '_';
}

bool isCharOfName(char ch)
{
    return std::isalnum(ch) || ch == '_';
}

bool isValidNumber(char ch, const std::string& number)
{
    if (std::isdigit(ch))
    {
        return true;
    }

    return (number.empty() && (ch == '-' || ch == '+')) ||
           (number == "0" && (ch == 'x' || ch == 'X'));
}

bool findToken(const std::string& text, size_t& pos, FoundToken& token)
{
    KMState state = KMState::TokenBegin;
    std::string tokenName = "";
    std::string value = "";

    while (pos < text.size() && state != KMState::Error && state != KMState::Success)
    {
#ifdef _DEBUG
        char ch = text[pos];
#endif
        switch (state)
        {
            case KMState::TokenBegin:
                removeWhitespace(text, pos);
                if (isFirstCharOfName(text[pos]))
                {
                    state = KMState::TokenEnd;
                    token.begPos = pos;
                }
                else
                {
                    ++pos;
                    state = KMState::Error;
                }
                break;

            case KMState::TokenEnd:
                if (isCharOfName(text[pos]))
                {
                    tokenName += text[pos++];
                }
                else
                {
                    token.info = getToken(tokenName);
                    state = token.info ? KMState::OpenBraceBegin : KMState::Error;
                }
                break;

            case KMState::OpenBraceBegin:
                removeWhitespace(text, pos);
                state = text[pos++] == '(' ? KMState::OpenBraceEnd : KMState::Error;
                break;

            case KMState::OpenBraceEnd:
                removeWhitespace(text, pos);
                state = std::isdigit(text[pos])
                    ? KMState::NumberBegin
                    : (token.info->argsCount ? KMState::Error : KMState::CloseBrace);
                break;

            case KMState::NumberBegin:
                removeWhitespace(text, pos);
                value = "";
                state = isValidNumber(text[pos], value) ? KMState::NumberEnd : KMState::Error;
                break;

            case KMState:: NumberEnd:
                if (isValidNumber(text[pos], value))
                {
                    value += text[pos++];
                }
                else
                {
                    if (value.size() >= 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
                    {
                        size_t u64 = 0;
                        state = su::String_IsValidHex(value.c_str() + 2, u64) ? KMState::SeparatorBegin : KMState::Error;
                        token.args.push_back(u64);
                    }
                    else
                    {
                        char** endPtr = nullptr;
                        token.args.push_back(std::strtoll(value.c_str(), endPtr, 10));
                        state = !endPtr ? KMState::SeparatorBegin : KMState::Error;
                    }
                }
                break;

            case KMState::SeparatorBegin:
                removeWhitespace(text, pos);
                if (text[pos] == ',')
                {
                    ++pos;
                    state = token.args.size() < token.info->argsCount ? KMState::NumberBegin : KMState::Error;
                }
                else if (text[pos] == ')')
                {
                    state = token.args.size() == token.info->argsCount ? KMState::CloseBrace : KMState::Error;
                }
                else
                {
                    state = KMState::Error;
                }
                break;

            case KMState::CloseBrace:
                removeWhitespace(text, pos);
                state = text[pos++] == ')' ? KMState::Success : KMState::Error;
                token.endPos = pos;
                break;
        }
    }

    return state == KMState::Success;
}

std::string doTokenize(const std::string& line, bool& found)
{
    found = false;

    if (line.empty())
    {
        return line;
    }

    size_t pos = 0;
    FoundToken token;
    std::string text = line;

    while (pos < text.size())
    {
        if (!findToken(text, pos, token))
        {
            continue;
        }

        std::string funcOut = token.info->function(token.args);
        char chB = text[token.begPos];
        char chE = text[token.endPos];
        text.erase(text.begin() + token.begPos, text.begin() + token.endPos);
        text.insert(token.begPos, funcOut);
        pos = token.begPos + funcOut.size();
        char chN = text[pos];
        found = true;
    }

    return text;
}

int doShuffle(const std::string& filename, bool replace, bool backup, bool weakrand)
{
    printf("Shuffling file: %s\n", filename.c_str());

    std::ifstream inFile(filename/*, std::ios_base::app*/);

    if (!inFile.is_open())
    {
        printf("ERROR: The '%s' file not open!\n", filename.c_str());
        return false;
    }

    std::stringstream inBuffer;
    std::stringstream outBuffer;
    std::vector<std::string> suffleLines;

    inBuffer << inFile.rdbuf();
    inFile.close();

    bool foundMarker = false;
    bool foundToken = false;
    std::string line;
    const std::string whitespace = " \t\r\n\f";
    while (std::getline(inBuffer, line))
    {
        line = doTokenize(line, foundToken);
        std::string clearLine = su::String_trim(line, whitespace);

        if (clearLine == "    ")
        {
            volatile int a = 1;
        }

        if (!foundMarker)
        {
            foundMarker = clearLine == global::begMarker;
            outBuffer << line << "\n";
        }
        else
        {
            if (clearLine == global::endMarker)
            {
                smartShuffle(suffleLines, weakrand);

                for (auto& item : suffleLines)
                {
                    outBuffer << item << "\n";
                }
                suffleLines.clear();
                outBuffer << line << "\n";
                foundMarker = false;
            }
            else
            {
                if (clearLine.size())
                {
                    suffleLines.push_back(line);
                }
            }
        }
    }

    std::string outputFilename = filename + std::string(".") + global::out;

    if (replace)
    {
        if (backup)
        {
            try
            {
                std::string backFilename = filename + ".back";

#ifdef _WIN32
                SetFileAttributes(backFilename.c_str(), GetFileAttributes(backFilename.c_str()) & ~FILE_ATTRIBUTE_READONLY);
#endif
                fs::copy(filename, backFilename, fs::copy_options::overwrite_existing);
            }
            catch (...)
            {
                printf("EXCEPTION: Can not create backup of '%s' file!\n", filename.c_str());
            }
        }
        outputFilename = filename;
    }

    try
    {
#ifdef _WIN32
        SetFileAttributes(outputFilename.c_str(), GetFileAttributes(outputFilename.c_str()) & ~FILE_ATTRIBUTE_READONLY);
#endif
        std::ofstream outFile(outputFilename);
        if (!outFile.is_open())
        {
            printf("ERROR: The '%s' output file can not be open!\n", outputFilename.c_str());
            return false;
        }

        outFile.write(outBuffer.str().c_str(), outBuffer.str().size());
        outFile.close();
        printf("Save file: %s\n", outputFilename.c_str());
    }
    catch (...)
    {
        printf("EXCEPTION: Can not save of '%s' file!\n", outputFilename.c_str());
    }

    return true;
}
