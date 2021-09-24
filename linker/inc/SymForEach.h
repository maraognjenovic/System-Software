#ifndef SYMFOREACH_H
#define SYMFOREACH_H
#include <vector>
#include <string>
#include "SymbolTable.h"
#include <map>

using namespace std;

class SymForEachFile{
public:
    map<string, map<string, SymbolTable>> symForEachFile;
    void setSymTable(const string s, map<string, SymbolTable> t){
        this->symForEachFile[s] = t;
    }
    map<string, SymbolTable> getSymTable(string s){
        return this->symForEachFile[s];
    }
};
#endif