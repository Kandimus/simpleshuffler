
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
const su::CommandLineOption HELP = { "help", 'h' };
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

    cl.addSwitch(arg::HELP)
        .addOption(arg::THEAD, "-1")
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

    if (files.empty() || cl.isSet(arg::HELP.first))
    {
        printf("A simple shuffler designed to shuffle lines in source code files (C) Toadman\n");
        printf("version 1.1\n");
        printf("usage: simpleshuffler [options and switches] files\n");
        printf("Options and switches:\n");
        printf("\t--thread,-t=<count>\t\t\trun on <count> threads\n");
        printf("\t--begmarker,-m=<new begin marker>\tset a new begin marker, default is '%s'\n",
               global::begMarker.c_str());
        printf("\t--endmarker,-M=<new end marker>\t\tset a new end marker, default is '%s'\n",
               global::endMarker.c_str());

        return 1;
    }

    //TODO create thread_manager

    for (auto& item : files)
    {
        doShuffle(item);
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

void smartShuffle(std::vector<std::string>& lines)
{
#ifndef _WIN32
    std::random_device rd;
    std::mt19937 g(rd());
#endif

    auto oldLines = lines;

    size_t old_index = 0;
    size_t new_index = 0;
    double dRand = double(lines.size()) / double(RAND_MAX);

    while (true)
    {
        
#ifdef _WIN32
        new_index = size_t(rand() * dRand);
#else
        new_index = g() * source.size() / RAND_MAX;
#endif

        if (new_index == old_index)
        {
            continue;
        }

        std::iter_swap(lines.begin() + old_index, lines.begin() + new_index);

        if (checkShufflingQuality(oldLines, lines))
        {
            break;
        }

        if (++old_index >= lines.size())
        {
            old_index = 0;
        }
    }
}

int doShuffle(const std::string& filename)
{
    printf("Shuffling file: %s", filename.c_str());

    std::ifstream inFile(filename/*, std::ios_base::app*/);

    if (!inFile.is_open())
    {
        printf("ERROR: inFile not open!");
        return false;
    }

    std::stringstream inBuffer;
    std::stringstream outBuffer;
    std::vector<std::string> suffleLines;

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
                smartShuffle(suffleLines);

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
