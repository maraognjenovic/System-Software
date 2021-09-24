#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <vector>
#include <map>
#include <regex>
#include <list>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include "SymbolTable.h"
#include "RelocationTable.h"
#include "SectionTable.h"
/*
#! /bin/sh
g++ -o asembler  src/*.cpp 
#! /bin/sh
./asembler -o interrupts.txt test/interrupts.s
./asembler -o main.txt test/main.s

chmod +x compile.sh
chmod +x run.sh
*/
const regex  globalDirective("^\\.global ([a-zA-Z][a-zA-Z0-9_]*(,[a-zA-Z][a-zA-Z0-9_]*)*)$");
const regex  externDirective("^\\.extern ([a-zA-Z][a-zA-Z0-9_]*(,[a-zA-Z][a-zA-Z0-9_]*)*)$");
const regex  sectionDirective("^\\.section ([a-zA-Z][a-zA-Z0-9_]*)$");
const regex  wordDirective("^\\.word (([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)(,([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+))*)$");
const regex  skipDirective("^\\.skip (-?[0-9]+|0x[0-9A-F]+)$");
const regex  equDirective("^\\.equ ([a-zA-Z][a-zA-Z0-9_]*),(-?[0-9]+|0x[0-9A-F]+)$");
const regex  endDirective("^\\.end$");
const regex  label("^([a-zA-Z][a-zA-Z0-9_]*):$");
const regex  labelFollowedBySmth("^([a-zA-Z][a-zA-Z0-9_]*):(.*)$");

const regex  symbol("^([a-zA-Z][a-zA-Z0-9_]*)$");
const regex  decimal("^(-?[0-9]+)$");
const regex  hexadecimal("^(0x[0-9A-F]+)$");
const regex  insHIR("^(halt|iret|ret)$");
const regex  insRegOneOp("^(push|pop|int|not) (r[0-7]|psw)$");
const regex  insRegTwoOp("^(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) (r[0-7]|psw),(r[0-7]|psw)$");
const regex  insJmpOneOp("^(call|jmp|jeq|jne|jgt) (.*)$");
const regex  insLdStTwoOp("^(ldr|str) (r[0-7]|psw),(.*)$");
const regex  jmpApsolute("^([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)$");
const regex jmpMemDir("^\\*([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)$");
const regex  jmpPcRelative("^%([a-zA-Z][a-zA-Z0-9_]*)$");
const regex  jmpRegDir("^\\*(r[0-7]|psw)$");
const regex  jmpRegInd("^\\*\\[(r[0-7]|psw)\\]$");
const regex  jmpRegIndShift("^\\*\\[(r[0-7]|psw) \\+ ([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)\\]$");
const regex  LdStApsolute("^\\$([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)$");
const regex  LdStMemDir("^([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)$");
const regex  LdStPcRelative("^%([a-zA-Z][a-zA-Z0-9_]*)$");
const regex  LdStRegDir("^(r[0-7]|psw)$");
const regex  LdStRegInd("^\\[(r[0-7]|psw)\\]$");
const regex  LdStRegIndShift("^\\[(r[0-7]|psw) \\+ ([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]+)\\]$");
const char nula='0';

using namespace std;

class Assembler{

private:
    
    list <string> inputLines;
    int cnt;
    int currNumOfLine;
    static int idSymbol;
    static int idSect;
    static int lineBefore;
    static int lineAfter;
    string currSect;
    string input;

    string pathOutput;
    bool error;
    map<string, SymbolTable> symbolTable;
    list<RelocationTable> relocationTable;
    map<string, SectionTable> sectionTable;
    map<int, string> errorNotification;
    map<int, int> realNumOfErrorLine;
    ofstream output;
    int firstPass();
    int secondPass();
    int removeExcess();
    void processLine(string &line);
    void makeText();
    int workOnAps(string);
    int workOnWordSecond(string);
    int workOnPCRelative(string);
    int workOnInstructionSecond(string);
    int workOnLabel(string);
    int workOnSection(string);
    int workOnGlobalSymbol(string);
    int workOnExternSymbol(string);
    int workOnEqu(string,string);
    int workOnSkipFirst(string);
    int workOnWordFirst();
    int getInstructionCode(string);
    void addSize(string,int);
    int errorDetected(int,string);
    int getValueFromString(string);
    int workOnInstructionFirst(string);
    void pushBackSection2(string, int, int );
    void pushBackSection(string,int, int, int);
    void pushBackSection3Data(string, int,int,int);
    bool cmp(pair<string, int>& a,pair<string, int>& b);
  map<SymbolTable, string>sort(map<string, SymbolTable> M);
    void makeBinary();
    void write(ofstream &, string );
    void addSymbToBinary(ofstream &);
    void addSecToBinary(ofstream &);
    void addRelToBinary(ofstream &);
public:
    Assembler(string, string);
    bool compile();
    bool errors();
};
#endif
