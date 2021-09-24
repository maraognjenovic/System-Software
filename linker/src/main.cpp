#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <regex>
#include "../inc/Linker.h"

using namespace std;

int main(int argc, const char *argv[])
{

    bool isOutput = false;
    string outputFileName;
    bool isPlace = false;
    smatch sectionAdrs;
    map<string, int> sectionAddresses;
    bool isLinkable = true;
    bool isHex = true;
    vector<string> inputFilesAll;
    int i = 1;
    while (i < argc)
    {
        string current = argv[i];
        switch (current == "-o")
        {
        case true:
            isOutput = true;
            break;

        case false:
            switch (current == "-hex")
            {
            case true:
                isLinkable = false;
                break;

            case false:
                switch (current == "-linkable")
                {
                case true:
                    isHex = false;
                    break;

                case false:
                    if (regex_search(current, sectionAdrs, regex("^-place=([a-zA-Z_][a-zA-Z_0-9]*)@(0[xX][0-9a-fA-F]+)$")))
                    {
                        isPlace = true;
                        sectionAddresses[sectionAdrs.str(1)] = stoi(sectionAdrs.str(2), nullptr, 16);
                    }
                    else switch (isOutput)
                    {
                        case true:
                        outputFileName = current;
                        isOutput = false;
                        break;
                        case false:
                        inputFilesAll.push_back(current);
                        break;
                    }
                    break;
                }
                break;
            }
            break;
        }
        i++;
    }

    if (!isHex && !isLinkable)
    {
        cout << "-linkable i -hex ne mogu se koristiti istovremeno!" << endl;
        return -1;
    }
    if (isHex && isLinkable)
    {
        cout << "Morate izabrati jednu od opcija -linkable or -hex!" << endl;

        return -1;
    }
    Linker linker(inputFilesAll, outputFileName, sectionAddresses, isHex);
    if (linker.errors())
        return -1;

    return 0;
}