#ifndef RELOCFOREACH_H
#define RELOCFOREACH_H
#include <vector>
#include <string>
#include "RelocTable.h"
#include <map>

using namespace std;

class RelocForEachFile{
public:
    map<string, vector<RelocTable>> relocForEachFile;
    void setRelocTable(const string s, vector<RelocTable> t){
        this->relocForEachFile[s] = t;
    }
    vector<RelocTable> getRelocTable(string s){
        return this->relocForEachFile[s];
    }
};
#endif