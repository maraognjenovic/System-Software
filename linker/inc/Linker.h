#ifndef LINKER_H
#define LINKER_H
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "SymbolTable.h"
#include "RelocTable.h"
#include "SectionTable.h"
#include "SectionAdd.h"
#include "SymForEach.h"
#include "RelocForEach.h"
#include "SecForEach.h"
const char nula = '0';
using namespace std;
/*
#! /bin/sh
g++ -o linker src/*.cpp 
#! /bin/sh
./linker -hex -place=ivt@0x0000 -o program.hex test/interrupts.o test/main.o

*/
class Linker
{
private:
    SymForEachFile symForEachFile;
    RelocForEachFile relocForEachFile;
    vector<RelocTable> outputRelocTable;
    map<string, int> sectionAddresses;
    vector<string> errorNotification;
    string outputFileName;
    bool isHex;
    int replaceSectToVma();
    int solveRelocLink();
    vector<string> inputFilesAll;
    int solveRelocHex();
    map<string, map<string, SectionAdditionalData>> sectionHelp;
    void read(ifstream &, string &);
    map<string, SymbolTable> outputSymTable;
    int readSymbTable(ifstream &inputFile, const string singleFile);
    map<string, SectionTable> outputSecTable;
    int readSecTable(ifstream &inputFile, const string singleFile);
    int readRelocTable(ifstream &inputFile, const string singleFile);
    map<string, map<string, SectionTable>> secForEachFile;

public:

    Linker(vector<string>,string, map<string, int>, bool);
    void makeText();
    int readDataFromBinary();
    int makeAgrSec();
    int makeAgrSymb();
    int makeAgrReloc();
    int makeAgrContSec();
    bool errors();
};
#endif //LINKER_H