#ifndef SECFOREACH_H
#define SECFOREACH_H
#include <vector>
#include <string>
#include "SectionTable.h"
#include <map>

using namespace std;

class SecForEachFile{
public:
    map<string, map<string, SectionTable>> secForEachFile;
    void setSecTable(const string s, map<string, SectionTable> t){
        this->secForEachFile[s] = t;
    }
    map<string, SectionTable> getSecTable(string s){
        return this->secForEachFile[s];
    }
    vector<int> getOffsets(string filename, string name){
        return this->secForEachFile[filename][name].offsets;
    }
        vector<char> getData(string filename, string name){
        return this->secForEachFile[filename][name].data;
    }

};
#endif