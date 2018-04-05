#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

#include "astree.h"
#include "lyutils.h"
#include "auxlib.h"

struct symbol {
   // variables are named as same as assignment requirements
   attr_bitset attributes;
   symbol_table* fields;
   location lloc; //include size_t filenr, linenr, offset
   size_t block_nr;
   vector<symbol*> parameters;
   // extra variables to assist symbol table
   const string* para_name;
   const string* para_type;
   const string symbol_attr_string(const string *struct_name);
};

int typecheck(astree *tree);

#endif
