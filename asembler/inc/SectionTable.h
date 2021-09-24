#ifndef SECTIONTABLE_H
#define SECTIONTABLE_H
#include <vector>
#include <string>
using namespace std;

class SectionTable
{
public:
    int idSection;
    int size;
    string name;

public:
    vector<char> data;
    vector<int> offsets;
    void addSize(int n)
    {
        size += n;
    }
    int getIdSection()
    {
        return this->idSection;
    }
    void setIdSection(int idSection)
    {
        this->idSection = idSection;
    }
    string getName()
    {
        return this->name;
    }
    void setName(string name)
    {
        this->name = name;
    }
    int getSize()
    {
        return this->size;
    }
    void setSize(int size)
    {
        this->size = size;
    }

    SectionTable(int, std::string, int);
    SectionTable() = default;
};

#endif
