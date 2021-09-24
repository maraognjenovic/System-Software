#ifndef SECTIONADD_H
#define SECTIONADD_H
#include <vector>
#include <string>
using namespace std;

class SectionAdditionalData{
        public:
        string section_name;
        string parent_file_name;
        int parentSecSize;
        int startAdrAgrSec;

        SectionAdditionalData()=default;
        SectionAdditionalData(string,string,int,int);

};
#endif