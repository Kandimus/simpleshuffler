
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

#include "commandline.h"
#include "stringex.h"

namespace arg
{
const su::CommandLineOption THEAD = {"thread", 't'};
const su::CommandLineOption BEGMARKER = {"begmarker", 'm'};
const su::CommandLineOption ENDMARKER = {"endmarker", 'M'};
}

namespace global
{
std::string begMarker = "//>>>BEGIN_SHUFFLING";
std::string endMarker = "//<<<END_SHUFFLING";
}

int doShuffle(const std::string& filename);

int main(int argc, const char** argv)
{
    su::CommandLine cl;

    cl.addOption(arg::THEAD, "-1")
        .addOption(arg::BEGMARKER, global::begMarker)
        .addOption(arg::ENDMARKER, global::endMarker)
        .parse(argc, argv);

    srand(time(0));

    global::begMarker = cl.getOption(arg::BEGMARKER.first);
    global::endMarker = cl.getOption(arg::ENDMARKER.first);

    std::vector<std::string> files;

    for (int ii = 0; ii < cl.getCountArgument(); ++ii)
    {
        files.push_back(cl.getArgument(ii));
    }

    //TODO create thread_manager

    for (auto& item : files)
    {
        doShuffle(item);
    }

    return 0;
}

int doShuffle(const std::string& filename)
{
    std::ifstream inFile(filename/*, std::ios_base::app*/);

    if (!inFile.is_open())
    {
        printf("ERROR: inFile not open!");
        return false;
    }

    std::stringstream inBuffer;
    std::stringstream outBuffer;
    std::vector<std::string> suffleLines;
#ifndef _WIN32
    std::random_device rd;
    std::mt19937 g(rd());
#endif

    inBuffer << inFile.rdbuf();
    inFile.close();

    bool foundMarker = false;
    std::string line;
    const std::string whitespace = " \t\r\n\f";
    while (std::getline(inBuffer, line))
    {
        std::string clearLine = su::String_trim(line, whitespace);

        if (!foundMarker)
        {
            foundMarker = clearLine == global::begMarker;
            outBuffer << line << "\n";
        }
        else
        {
            if (clearLine == global::endMarker)
            {
#ifdef _WIN32
                std::random_shuffle(suffleLines.begin(), suffleLines.end());
#else
                std::random_shuffle(suffleLines.begin(), suffleLines.end(), g);
#endif
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
                suffleLines.push_back(line);
            }
        }
    }

    std::ofstream outFile(filename + ".out");
    if (!outFile.is_open())
    {
        printf("ERROR: outFile not open!");
        return false;
    }
    outFile.write(outBuffer.str().c_str(), outBuffer.str().size());
    outFile.close();

    return true;
}
