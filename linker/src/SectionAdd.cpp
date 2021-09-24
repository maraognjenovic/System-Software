#include "../inc/SectionAdd.h"
SectionAdditionalData::SectionAdditionalData(string section_name,string parent_file_name,int parentSecSize,int startAdrAgrSec){
    this->section_name=section_name;
    this->parent_file_name=parent_file_name;
    this->parentSecSize=parentSecSize;
    this->startAdrAgrSec=startAdrAgrSec;
}