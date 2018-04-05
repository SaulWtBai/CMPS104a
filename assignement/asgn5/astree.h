#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
using namespace std;

#include "auxlib.h"

struct location {
  size_t filenr;
  size_t linenr;
  size_t offset;
};


enum { ATTR_void, ATTR_int, ATTR_null, ATTR_string,
       ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
       ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
       ATTR_vreg, ATTR_vaddr, ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<const string*, symbol*>;
using symbol_entry = pair<const string*,symbol*>;

struct astree {

   // Fields.
   int symbol;               // token code
   location lloc;            // source location
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node

   //-------hw4-----
   attr_bitset attributes;
   size_t block_nr;
   location definedl;
   const string *treeType;
   string oilname;//---------hw5
   int global;//---------hw5

   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   ~astree();
   //---hw3
   astree* adopt (astree* child1, astree* child2 = nullptr);
   astree* adopt_sym (astree* child, int symbol);
   astree* forwardAdopt (astree* forward);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
   string astree_string_attr();
   string treeTypeFunc();
};

void free_ast (astree* tree1, astree* tree2 = nullptr);
void errllocprintf (const location&, const char* format, const char*);

#endif

