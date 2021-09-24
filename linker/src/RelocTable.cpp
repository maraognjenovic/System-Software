#include "../inc/RelocTable.h"
RelocTable::RelocTable(bool data, string name, int offset, string type, int addend,string symbolName,string filename):
data(data),name(name),offset(offset),type(type),addend(addend),symbolName(symbolName),filename(filename)
{}

bool RelocTable::getData(){
    return this->data;
}
string RelocTable::getSectionName(){
    return this->name;
}
int RelocTable::getOffset(){
    return this->offset;
}

string RelocTable::getType(){
    return type;
}
string RelocTable::getSymbolName(){
    return this->symbolName;
}
int RelocTable::getAddend(){
    return this->addend;
}
void RelocTable::setData(bool data){
    this->data=data;
}
void RelocTable::setSectionName(string name){
    this->name=name;
}
void RelocTable::setOffset(int offset){
    this->offset=offset;
}
void RelocTable::setType(string type){
    this->type=type;
}
void RelocTable::setAddend(int addend){
    this->addend=addend;
}
void RelocTable::setSymbolName(string symbolName){
    this->symbolName=symbolName;
}