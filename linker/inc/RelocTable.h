#ifndef RelocTable_H
#define RelocTable_H
#include <string>
using namespace std;
class RelocTable
        {
public:
    bool data;
    string name;
    int offset;
    string type;
    int addend;
    string symbolName;
    string filename; 

        public:
            RelocTable() = default;
            RelocTable(bool, string, int, string, int, string,string);
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
