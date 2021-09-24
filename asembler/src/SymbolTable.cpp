#include "../inc/SymbolTable.h"
SymbolTable::SymbolTable(int id_symbol, bool defined, bool is_extern, bool binding, string name, string section, int value):
id_symbol(id_symbol),defined(defined),is_extern(is_extern),binding(binding),name(name),section(section),value(value)
{}
