#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_dump->intern(info);
   // vector defaults to empty -- no children
   block_nr = -1;
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   return this;
}


astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}

//---hw3----adopt forward child
astree* astree::forwardAdopt (astree* forward) {
   if (forward != nullptr) {
      vector<astree*>::iterator head;
      head = children.begin();
      children.insert(head, forward);
   }
   return this;
}

void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
             this, parser::get_tname (symbol),
             lloc.filenr, lloc.linenr, lloc.offset,
             lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   for(int i = 0; i < depth; i++) {
      fprintf(outfile, "|\t");
   }
   char* tname = (char*) parser::get_tname(tree->symbol);
   if(strstr(tname, "TOK_")==tname) tname+=4;

   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd)\n",
            parser::get_tname (tree->symbol), tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
   
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

//-----
string astree::astree_string_attr(){
   string temp = "";  
   if (attributes.test (ATTR_void)) temp += "void ";
   if (attributes.test (ATTR_int)) temp += "int ";
   if (attributes.test (ATTR_null)) temp += "null ";
   if (attributes.test (ATTR_string)) temp += "string ";
   if (attributes.test (ATTR_struct))
      temp = temp+"struct "+"\""+*treeType+"\" ";
   if (attributes.test (ATTR_array)) temp += "array ";
   if (attributes.test (ATTR_function)) temp += "function ";
   if (attributes.test (ATTR_variable)) temp += "variable ";
   if (attributes.test (ATTR_field)) temp += "field ";
   if (attributes.test (ATTR_typeid)) temp += "typeid ";
   if (attributes.test (ATTR_param)) temp += "param ";
   if (attributes.test (ATTR_lval)) temp += "lval ";
   if (attributes.test (ATTR_const)) temp += "const ";
   if (attributes.test (ATTR_vreg)) temp += "vreg ";
   if (attributes.test (ATTR_vaddr)) temp += "vaddr ";
   if (symbol == TOK_IDENT) {
      temp=temp+"("+to_string(definedl.filenr)+"."
      +to_string(definedl.linenr)+"."+to_string(definedl.offset)+")";
   }
   return temp;
}

//string astree::type_string() {
string astree::treeTypeFunc() {
   string temp = "";  
   if (attributes.test (ATTR_void)) temp += "void ";
   if (attributes.test (ATTR_int)) temp += "int ";
   if (attributes.test (ATTR_null)) temp += "null ";
   if (attributes.test (ATTR_string)) temp += "string ";
   if (attributes.test (ATTR_struct))
      temp = temp+"struct "+"\""+*treeType+"\" ";
   if (attributes.test (ATTR_array)) temp += "array";
   return temp;

}

void free_ast (astree* tree1, astree* tree2) { 
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr)->c_str(), 
              lloc.linenr, lloc.offset, buffer);
}



