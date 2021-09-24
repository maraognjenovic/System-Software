#include "../inc/RelocationTable.h"
RelocationTable::RelocationTable(bool data, string name, int offset, string type, int addend,string symbolName):
data(data),name(name),offset(offset),type(type),addend(addend),symbolName(symbolName){}

bool RelocationTable::getData(){
    return this->data;
}
string RelocationTable::getSectionName(){
    return this->name;
}
int RelocationTable::getOffset(){
    return this->offset;
}

string RelocationTable::getType(){
    return type;
}
string RelocationTable::getSymbolName(){
    return this->symbolName;
}
int RelocationTable::getAddend(){
    return this->addend;
}
void RelocationTable::setData(bool data){
    this->data=data;
}
void RelocationTable::setSectionName(string name){
    this->name=name;
}
void RelocationTable::setOffset(int offset){
    this->offset=offset;
}
void RelocationTable::setType(string type){
    this->type=type;
}
void RelocationTable::setAddend(int addend){
    this->addend=addend;
}
void RelocationTable::setSymbolName(string symbolName){
    this->symbolName=symbolName;
}