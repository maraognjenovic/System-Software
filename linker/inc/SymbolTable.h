#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <string>
using namespace std;

class SymbolTable{
public:
    int id_symbol; 
    bool binding; 
    bool defined;
    bool is_extern;
    int value;
    string section;
    string name; 
public:
void editSizeName(int n){
    this->name.resize(n);
}
void editSizeSection(int n){
    this->section.resize(n);
}
 bool getIs_extern() {
 	return this->is_extern;
 }
 void setIs_extern(bool is_extern) {
 	this->is_extern = is_extern;
 }
string getSection() {
	return this->section;
}
void setSection(string section) {
	this->section = section;
}
string getName() {
	return this->name;
}
void setName(string name) {
	this->name = name;
}
 int getValue() {
 	return this->value;
 }
 void setValue(int value) {
 	this->value = value;
 }

bool getDefined() {
	return this->defined;
}
void setDefined(bool defined) {
	this->defined = defined;
}
    bool getBinding() {
    	return this->binding;
    }
    void setBinding(bool binding) {
    	this->binding = binding;
    }
    int getId_symbol() {
    	return this->id_symbol;
    }
    void setId_symbol(int id_symbol) {
    	this->id_symbol = id_symbol;
    }

    SymbolTable()=default;
    SymbolTable(int,bool,bool, bool, string,string,int);

};
#endif
