
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <random>

#include <windows.h>

#include "commandline.h"
#include "stringex.h"

namespace fs = std::filesystem;

namespace arg
{
const su::CommandLineOption HELP = { "help", 'h' };
const su::CommandLineOption REPLACE = { "replace", 'r' };
const su::CommandLineOption NOBACKUP = { "nobackup", 'B' };
const su::CommandLineOption OUTPUT = { "out", 'o' };
const su::CommandLineOption THEAD = {"thread", 't'};
const su::CommandLineOption BEGMARKER = {"begmarker", 'm'};
const su::CommandLineOption ENDMARKER = {"endmarker", 'M'};
}

namespace global
{
std::string begMarker = "//>>>BEGIN_SHUFFLING";
std::string endMarker = "//<<<END_SHUFFLING";
std::string out = "out";
}

int doShuffle(const std::string& filename, bool replace, bool nobackup);

int main(int argc, const char** argv)
{
    su::CommandLine cl;

    cl.addSwitch(arg::HELP)
        .addSwitch(arg::REPLACE)
        .addSwitch(arg::NOBACKUP)
        .addOption(arg::OUTPUT, global::out)
        .addOption(arg::THEAD, "-1")
        .addOption(arg::BEGMARKER, global::begMarker)
        .addOption(arg::ENDMARKER, global::endMarker)
        .parse(argc, argv);

    srand(time(0));

    global::begMarker = cl.getOption(arg::BEGMARKER.first);
    global::endMarker = cl.getOption(arg::ENDMARKER.first);
    global::out = cl.getOption(arg::OUTPUT.first);

    std::vector<std::string> files;

    for (int ii = 0; ii < cl.getCountArgument(); ++ii)
    {
        files.push_back(cl.getArgument(ii));
    }

    if (files.empty() || cl.isSet(arg::HELP.first))
    {
        printf("A simple shuffler designed to shuffle lines in source code files (C) Toadman\n");
        printf("version 1.4\n");
        printf("usage: simpleshuffler [options and switches] files\n");
        printf("Options and switches:\n");
        printf("\t--thread,-t=<count>\t\trun on <count> threads\n");
        printf("\t--replace,-r\t\t\tcreate backup file and replace source file\n");
        printf("\t--nobackup,-B\t\t\tdo not create backup file\n");
        printf("\t--out=<extension>\t\tset a new output extension, default is '%s'.\n"
            "\t\t\t\t\tIgnoring if `--replase` switch is present.\n",
            global::out.c_str());
        printf("\t--begmarker=<new begin marker>\tset a new begin marker, default is '%s'\n",
            global::begMarker.c_str());
        printf("\t--endmarker=<new end marker>\tset a new end marker, default is '%s'\n",
            global::endMarker.c_str());

        return 1;
    }

    //TODO create thread_manager

    for (auto& item : files)
    {
        doShuffle(item, cl.isSet(arg::REPLACE.first), !cl.isSet(arg::NOBACKUP.first));
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

int doShuffle(const std::string& filename, bool replace, bool backup)
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
