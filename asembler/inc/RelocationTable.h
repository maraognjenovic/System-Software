#ifndef RelocationTable_H
#define RelocationTable_H
#include <string>
using namespace std;
class RelocationTable
        {

public:    bool data;
    string name;
    int offset;
    string type;
    int addend;
    string symbolName;

        public:
            RelocationTable() = default;
            RelocationTable(bool, string, int, string, int, string);
            bool getData();
            string getSectionName();
            int getOffset();
            string getType();
            string getSymbolName();
            int getAddend();
            void setData(bool);
            void setSectionName(string);
            void setOffset(int);
            void setType(string);
            void setSymbolName(string);
            void setAddend(int);
        };

#endif
