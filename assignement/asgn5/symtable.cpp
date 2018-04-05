#include <string>
#include <unordered_map>
#include <bitset>
#include <iostream>
#include <vector>

#include "symtable.h"
#include "astree.h"
#include "lyutils.h"
#include "auxlib.h"


vector<symbol_table*> symbol_stack;
vector<int> block_stack;
size_t next_block = 1;
int current_indent = 0;
int returned = 0;
int typecheck_assistor(astree *node);
symbol *find_symbol(symbol_table *table, astree *node);
int return_attr = ATTR_void;
const string *return_struct;

symbol_table *struct_table;

const string symbol::symbol_attr_string(
const string *name = nullptr){
   string temp ="";
   if(attributes.test(ATTR_field))
      temp += "field ";
      if(name != nullptr) temp =temp+ "{"+*name+"} ";
   if (attributes.test (ATTR_void)) temp += "void ";
   if (attributes.test (ATTR_int)) temp += "int ";
   if (attributes.test (ATTR_null)) temp += "null ";
   if (attributes.test (ATTR_string)) temp += "string ";
   if (attributes.test (ATTR_struct))
      temp = temp+"struct "+"\""+*para_type+"\" ";
   if (attributes.test (ATTR_array)) temp += "array ";
   if (attributes.test (ATTR_function)) temp += "function ";
   if (attributes.test (ATTR_variable)) temp += "variable ";
   if (attributes.test (ATTR_typeid)) temp += "typeid ";
   if (attributes.test (ATTR_param)) temp += "param ";
   if (attributes.test (ATTR_lval)) temp += "lval ";
   if (attributes.test (ATTR_const)) temp += "const ";
   if (attributes.test (ATTR_vreg)) temp += "vreg ";
   if (attributes.test (ATTR_vaddr)) temp += "vaddr ";
   return temp;
}

// a new block increase next block counter
// and push nullpter to symboll stack
void push_block(){
  block_stack.push_back(next_block);
  symbol_stack.push_back(nullptr);
  next_block++;
}

// pop symbol stack and a block
void pop_block() {
  block_stack.pop_back();
  symbol_stack.pop_back();
}

int check_int_type(astree *node){
   if(!node->attributes.test(ATTR_int))
      return 0;
   if(node->attributes.test(ATTR_void))
      return 0;
   if(node->attributes.test(ATTR_null))
      return 0;
   if(node->attributes.test(ATTR_string))
      return 0;
   if(node->attributes.test(ATTR_struct))
      return 0;
   return 1;
}

// check whether the type is valid
int check_type_valid(astree *node){
   int type = 0;
   for(int i = 0; i < ATTR_array; i++){
      if(node->attributes.test(i)){
         if(type == 1)
            return 0;
         type = 1;
      }
   }
   return type;
}

// chech compatible of attributes
int check_attr_compatible(attr_bitset attr1, attr_bitset attr2){
   if(!attr1.test(ATTR_int)){
      if(attr2.test(ATTR_null))
         return 1;
   }
   if(!attr2.test(ATTR_int)){
      if(attr1.test(ATTR_null))
         return 1;
   }
   for(int i = 0; i < ATTR_function; i++){
      if(attr1.test(i) != attr2.test(i)){
         printf("%i\n", i);
         return 0;
      }
   }
   if(attr1.test(ATTR_array) != attr2.test(ATTR_array))
      return 0;
   return 1;
}

// check if attributes of attributes
int check_same_attr(attr_bitset attr1, attr_bitset attr2){
   for(int i = 0; i < ATTR_function; i++){
      if(attr1.test(i) != attr2.test(i))
         return 0;
   }
   if(attr1.test(ATTR_array) != attr2.test(ATTR_array))
      return 0;
   return 1;
}

// chech compatible of astree
int check_astree_compatible(astree *node1, astree *node2){
   if(!check_type_valid(node1) || !check_type_valid(node2)){
      return 0;
   }
   if(!check_int_type(node1)){
      if(node2->attributes.test(ATTR_null)){
         return 1;
      }
   }
   if(!check_int_type(node2)){
      if(node1->attributes.test(ATTR_null)){
         return 1;
      }
   }
   for(int i = 0; i < ATTR_function; i++){
      if (node1->attributes.test(i) != node2->attributes.test(i)){
         return 0;
      }
   }
   if(node1->attributes.test(ATTR_array) !=
   node2->attributes.test(ATTR_array)){
      return 0;
   }
   return 1;
}

// insert an identifier onto symbol stack
void insert_identif(const string *lexinfo, symbol *symbol){
   if(symbol_stack.back() == nullptr){
      symbol_stack.back() = new symbol_table;
   }
   symbol_stack.back()->insert(symbol_entry(lexinfo, symbol));
}

// Creates a new symbol from the astree node
symbol *new_symbol(astree *node){
   symbol *symbol_new = new symbol();
   symbol_new->attributes = node->attributes;
   symbol_new->fields = nullptr;
   symbol_new->lloc = node->lloc;
   symbol_new->block_nr = node->block_nr;
   if (node->treeType != nullptr) {
      symbol_new->para_type = node->treeType;
   }
   return symbol_new;
}

// checking whether an identifier is defined
symbol *find_identer(astree *node){
   for(auto it = symbol_stack.rbegin(); 
      it != symbol_stack.rend(); ++it){
      symbol *symbol = find_symbol(*it, node);
      if(symbol != nullptr){
         return symbol;
      }
   }
   return nullptr;
}

// checking whether a symbol table is defined
symbol *find_symbol(symbol_table *table, astree *node){
  if(table == nullptr){
      return nullptr;
   }
   // iterates through the symbol stack from top to bottom
   auto find = table->find(node->lexinfo);
   if(find != table->end()){
      // returns the the most local if found
      return find->second;
   }else{
      // return nullptr if not defined
      return nullptr;
   }
}

// let parameter type in symbol table to inherit node type in astree
void inher_type(symbol *symbol, astree *node){
   for(int i = 0; i < ATTR_function; i++){
      if(node->attributes.test(i)){
         symbol->attributes.set(i);
         if(i == ATTR_struct){
            symbol->para_type = node->treeType;
         }
      }
   }
}

// let node type in astree to inherit parameter type in symbol table
void inher_type(astree *node, symbol *symbol){
   for(int i = 0; i < ATTR_function; i++){
      if(symbol->attributes.test(i)){
         node->attributes.set(i);
         if(i == ATTR_struct){
            node->treeType = symbol->para_type;
         }
      }
   }
}

// let node type in astree to inherit parameter type from its child
void inher_type(astree *parent, astree *child){
   for(int i = 0; i < ATTR_function; i++){
      if(child->attributes.test(i)){
         parent->attributes.set(i);
         if(i == ATTR_struct){
            parent->treeType = child->treeType;
         }
      }
   }
}

// let all attributes in symbol table
// to inherit all attributes in astree
void inher_attributes(symbol *symbol, astree *node){
   for(int i = 0; i < ATTR_bitset_size; i++){
      if(node->attributes.test(i)){
         symbol->attributes.set(i);
         if(i == ATTR_struct){
            symbol->para_type = node->treeType;
         }
      }
   }
}

// let all attributes in astree
// to inherit all attributes in symbol table
void inher_attributes(astree *node, symbol *symbol){
   for(int i = 0; i < ATTR_bitset_size; i++){
      if(symbol->attributes.test(i)){
         node->attributes.set(i);
         if(i == ATTR_struct){
            node->treeType = symbol->para_type;
         }
      }
   }
}

// let all attributes in astree
// to inherit all attributes from its child
void inher_attributes(astree *parent, astree *child){
   for(int i = 0; i < ATTR_bitset_size; i++){
      if(child->attributes.test(i)){
         parent->attributes.set(i);
         if (i == ATTR_struct){
            parent->treeType = child->treeType;
         }
      }
  }
}

// print symbol table as same as assignment4 requirement
void print_sym(const string *lexdata, location locate,
   const string attr_str, int block_num = -1){
   if(block_stack.back() == 0 && next_block > 1)
      fprintf(symprint, "\n");
   for(int i = 0; i < current_indent; i++)
      fprintf(symprint, "   ");
   fprintf(symprint, "%s (%zd.%zd.%zd) ",
         lexdata->c_str(), locate.filenr, locate.linenr, locate.offset);
   if(block_num > -1)
      fprintf(symprint, "{%i} ", block_num);
   fprintf(symprint, "%s\n", attr_str.c_str());
}

// process struct declaration
int process_struct(astree *node){
   int errors = 0;
   if(block_stack.back() != 0){
      errllocprintf (node->lloc, 
      "error: structs wrong\n" ,
      node->lexinfo->c_str());
      return 1;
   }

   astree *type = node->children[0];
   type->attributes.set(ATTR_struct);
   type->treeType = type->lexinfo;
  
   auto find = struct_table->find(type->lexinfo);
   symbol *symbol_struct;

   if(find != struct_table->end()){
      symbol_struct = find->second;
      if(symbol_struct != nullptr) {
         if(symbol_struct->fields != nullptr){
            errllocprintf (node->lloc, 
            "error: \'%s\' wrong\n",
            type->lexinfo->c_str());
            return errors + 1;
         } 
      }
   }

   symbol_struct = new_symbol(node);
   symbol_struct->attributes = type->attributes;

   struct_table->insert(symbol_entry(type->lexinfo, symbol_struct));
   symbol_struct->fields = new symbol_table;
   symbol_struct->para_type = type->treeType;

   print_sym(type->lexinfo,type->lloc,
      type->astree_string_attr(),type->block_nr);
      
   current_indent++;
   astree *declarer_tree;
   astree *base_type_tree;
   astree *declid_tree;
   symbol *symbol_insert;
   for(size_t i = 1; i < node->children.size(); i++){
      declarer_tree = node->children[i];
      if(declarer_tree->symbol == TOK_ARRAY){
         declarer_tree->attributes.set(ATTR_array);
         base_type_tree = declarer_tree->children[0];
         symbol_insert = new_symbol(base_type_tree);
         symbol_insert->attributes.set(ATTR_array);
         declid_tree = declarer_tree->children[1];
      }else{
         base_type_tree = declarer_tree;
         symbol_insert = new_symbol(base_type_tree);
         declid_tree = declarer_tree->children[0];
      }
    
      declid_tree->attributes.set(ATTR_field);
    
      if(errors > 0){
         return errors;
      }
      switch(base_type_tree->symbol){
         case TOK_VOID:{
            errllocprintf(node->lloc, 
            "error: \'%s\'type void wrong'\n",
            declid_tree->lexinfo->c_str());
            base_type_tree->attributes.set(ATTR_void);
            return errors + 1;
            break;
         }
         case TOK_INT:{
            base_type_tree->attributes.set(ATTR_int);
            inher_type(symbol_insert, base_type_tree);
            inher_attributes(symbol_insert, declid_tree);
            symbol_struct->fields->insert(
            symbol_entry(declid_tree->lexinfo, symbol_insert));
            print_sym(declid_tree->lexinfo,
               symbol_insert->lloc,
               symbol_insert->symbol_attr_string(type->lexinfo));
            break;
         }
         case TOK_STRING:{
            base_type_tree->attributes.set(ATTR_string);
            inher_type(symbol_insert, base_type_tree);
            inher_attributes(symbol_insert, declid_tree);
            symbol_struct->fields->insert(
               symbol_entry(declid_tree->lexinfo, symbol_insert));
            print_sym(declid_tree->lexinfo,
               symbol_insert->lloc,
               symbol_insert->symbol_attr_string(type->lexinfo));
            break;
         }
         case TOK_TYPEID:{
            base_type_tree->attributes.set(ATTR_struct);
            base_type_tree->treeType = base_type_tree->lexinfo;
         symbol *struct_find = find_symbol(
                                 struct_table, base_type_tree);
         if (struct_find == nullptr) {
            struct_table->insert(
            symbol_entry(base_type_tree->lexinfo, nullptr));
         } 
         inher_type(symbol_insert, base_type_tree);
         inher_attributes(symbol_insert, declid_tree);
         symbol_struct->fields->insert(
            symbol_entry(declid_tree->lexinfo, symbol_insert));
         print_sym(declid_tree->lexinfo,
            symbol_insert->lloc,
            symbol_insert->symbol_attr_string(type->lexinfo));
         break;
         }
      }
   }
  
  current_indent--;
  return errors;
}

int process_vardecl(astree *node){
   int errors = 0;
   astree *left_child = nullptr;
   astree *right_child = nullptr;
   if (node->children.size() == 2) {
      left_child = node->children[0];
      right_child = node->children[1];
   }
  
   astree *declarer_tree = left_child;
   astree *base_type_tree;
   astree *declid_tree;

   if(declarer_tree->symbol == TOK_ARRAY){
      declarer_tree->attributes.set(ATTR_array);
      base_type_tree = left_child->children[0];
      declid_tree = left_child->children[1];
   }else{
      base_type_tree = left_child;
      declid_tree = left_child->children[0];
   }
  
   node->block_nr = block_stack.back();
   declarer_tree->block_nr = block_stack.back();
   base_type_tree->block_nr = block_stack.back();
   declid_tree->block_nr = block_stack.back();

   switch(base_type_tree->symbol){
      case TOK_VOID:{
         errllocprintf (node->lloc, 
         "error: \'%s\' type void wrong '\n",
         declid_tree->lexinfo->c_str());
         errors++;
         break;
      }
      case TOK_INT:{
         base_type_tree->attributes.set(ATTR_int);
         break;
      }
      case TOK_STRING:{
         base_type_tree->attributes.set(ATTR_string);
         break;
      }
      case TOK_TYPEID:{
         auto find_struct = struct_table->find(base_type_tree->lexinfo);
         if (find_struct == struct_table->end()) {
            errllocprintf (base_type_tree->lloc, 
            "error: struct \'%s\' wrong\n", 
                        base_type_tree->lexinfo->c_str());
            errors++;
         }else if(find_struct->second == nullptr){
            errllocprintf (base_type_tree->lloc,
            "error: type wrong \'%s\'\n",
            base_type_tree->lexinfo->c_str());
            errors++;
         }
         base_type_tree->attributes.set(ATTR_struct);
         base_type_tree->treeType = base_type_tree->lexinfo;
         declarer_tree->attributes.set(ATTR_struct);
         declarer_tree->treeType = left_child->lexinfo;
         break;
      }
   }
   errors += typecheck_assistor(right_child);
   inher_type(declarer_tree, base_type_tree);

   int compatible = check_astree_compatible(declarer_tree, right_child);
  
   if(!compatible){
      errllocprintf(node->lloc, 
      "error: type %s wrong\n", 
      declarer_tree->treeTypeFunc().c_str());
      errors++;
   }
   symbol *symbol_insert = find_identer(declid_tree);
   if(symbol_insert != nullptr){
      errllocprintf (declid_tree->lloc, 
      "error: variable \'%s\' over defined \n",
      declid_tree->lexinfo->c_str());
      errors++;
   }
   if(errors == 0){
      symbol_insert = new_symbol(declarer_tree);
      symbol_insert->attributes.set(ATTR_variable);
      symbol_insert->attributes.set(ATTR_lval);
      insert_identif(declid_tree->lexinfo, symbol_insert);
      print_sym(declid_tree->lexinfo,
      symbol_insert->lloc,
      symbol_insert->symbol_attr_string(),
      symbol_insert->block_nr);
   }
   return errors;
}

int process_function(astree *node){
   int errors = 0;
   int needs_matching = 0;
   if(block_stack.back() != 0){
      errllocprintf(node->lloc, 
      "error: wrong scope\n" ,
                  node->lexinfo->c_str());
      return 1;
   } 
   
   astree *return_type = node->children[0];
   astree *base_type_tree;
   astree *declid_tree;

   if(return_type->symbol == TOK_ARRAY){
      return_type->attributes.set(ATTR_array);
      base_type_tree = return_type->children[0];
      declid_tree = return_type->children[1];
   }else{
      base_type_tree = return_type;
      declid_tree = return_type->children[0];
   }
   
   node->block_nr = block_stack.back();
   return_type->block_nr = block_stack.back();
   base_type_tree->block_nr = block_stack.back();
   declid_tree->block_nr = block_stack.back();
   
   astree *parameters_tree = node->children[1];
   astree *block = nullptr;

   node->block_nr = block_stack.back();

   if(node->symbol == TOK_FUNCTION){
      block = node->children[2];
   }

   symbol *function_symbol = find_identer(declid_tree);
  
   if(function_symbol != nullptr){
      if(node->symbol == TOK_FUNCTION){
         if(function_symbol->attributes.test(ATTR_function)){
            errllocprintf (node->lloc, 
            "error: \'%s\' declare wrong\n", 
                     declid_tree->lexinfo->c_str());
            return errors + 1;
         }else{
            needs_matching = 1;
         }
      }else{
         errllocprintf(node->lloc, 
          "error: \'%s\' declare wrong\n", 
                     declid_tree->lexinfo->c_str());
         return errors + 1;
      }
   }

   switch(base_type_tree->symbol){
      case TOK_VOID:{
         base_type_tree->block_nr = block_stack.back();
         declid_tree->block_nr = block_stack.back();
         base_type_tree->attributes.set(ATTR_void);
         return_attr = ATTR_void;
         break;
      }
      case TOK_STRING:{
         base_type_tree->block_nr = block_stack.back();
         declid_tree->block_nr = block_stack.back();
         base_type_tree->attributes.set(ATTR_string);
         return_attr = ATTR_string;
         break;
      }
      case TOK_INT:{
         base_type_tree->block_nr = block_stack.back();
         declid_tree->block_nr = block_stack.back();
         base_type_tree->attributes.set(ATTR_int);
         return_attr = ATTR_int;
         break;
      }
      case TOK_TYPEID:{
         base_type_tree->block_nr = block_stack.back();
         declid_tree->block_nr = block_stack.back();
         base_type_tree->attributes.set(ATTR_struct);
         base_type_tree->treeType = base_type_tree->lexinfo;
         return_attr = ATTR_struct;
         return_struct = declid_tree->lexinfo;
         break;
      }
   }
   
   inher_type(return_type, base_type_tree);

   if(function_symbol == nullptr){
      function_symbol = new_symbol(return_type);
      function_symbol->attributes.set(ATTR_function);
      insert_identif(declid_tree->lexinfo, function_symbol);
      print_sym(declid_tree->lexinfo,
            function_symbol->lloc,
            function_symbol->symbol_attr_string(),
            function_symbol->block_nr);
      push_block();
      current_indent++;
      parameters_tree->block_nr = block_stack.back();
      astree *param_declarer_tree;
      astree *param_basetype_tree;
      astree *param_declid_tree;
      symbol *param_symbol_tree;

      for(size_t i = 0; i < parameters_tree->children.size(); i++){
         param_declarer_tree = parameters_tree->children[i];
         if(param_declarer_tree->symbol == TOK_ARRAY){
            param_declarer_tree->attributes.set(ATTR_array);
            param_basetype_tree = param_declarer_tree->children[0];
            param_declid_tree = param_declarer_tree->children[1];
         }else{
            param_basetype_tree = param_declarer_tree;
            param_declid_tree = param_declarer_tree->children[0];
         }
         errors += typecheck_assistor(param_declarer_tree);
         errors += typecheck_assistor(param_basetype_tree);
         inher_type(param_declarer_tree, param_basetype_tree);
         param_declarer_tree ->block_nr = block_stack.back();
         param_declid_tree ->block_nr = block_stack.back();
         param_basetype_tree ->block_nr = block_stack.back();
         param_declarer_tree->attributes.set(ATTR_param);
         param_declarer_tree->attributes.set(ATTR_variable);
         param_declarer_tree->attributes.set(ATTR_lval);
         param_symbol_tree = new_symbol(param_declarer_tree);
         param_symbol_tree->para_type = param_declid_tree->lexinfo;
         if(errors == 0){
            insert_identif(param_declid_tree->lexinfo,
                           param_symbol_tree);
            function_symbol->parameters.push_back(param_symbol_tree);
            print_sym(param_declid_tree->lexinfo,
                  param_symbol_tree->lloc,
                  param_symbol_tree->symbol_attr_string(),
                  param_symbol_tree->block_nr);
         }
      }
   }

   if(node->symbol == TOK_FUNCTION){ 
      function_symbol->attributes.set(ATTR_function);
      if(needs_matching){
         if(parameters_tree->children.size() !=
            function_symbol->parameters.size()){
               errllocprintf (node->lloc,
                        "error: wrong arguments\n",
                       declid_tree->lexinfo->c_str());
               return errors + 1;
            }
         print_sym(declid_tree->lexinfo,
                declid_tree->lloc,
                function_symbol->symbol_attr_string(),
                declid_tree->block_nr);
         push_block();
         current_indent++;
         parameters_tree->block_nr = block_stack.back();
         astree *param_declarer_tree;
         astree *param_basetype_tree;
         astree *param_declid_tree;
         symbol *param_symbol_tree;
         symbol *param_check;

         for(size_t i = 0; i < parameters_tree->children.size(); i++){
            param_declarer_tree = parameters_tree->children[i];
            if(param_declarer_tree->symbol == TOK_ARRAY){
               param_declarer_tree->attributes.set(ATTR_array);
               param_basetype_tree = param_declarer_tree->children[0];
               param_declid_tree = param_declarer_tree->children[1];
            }else{
               param_basetype_tree = param_declarer_tree;
               param_declid_tree = param_declarer_tree->children[0];
            }
            errors += typecheck_assistor(param_declarer_tree);
            errors += typecheck_assistor(param_basetype_tree);
            inher_type(param_declarer_tree, param_basetype_tree);
            param_declarer_tree ->block_nr = block_stack.back();
            param_declid_tree ->block_nr = block_stack.back();
            param_basetype_tree ->block_nr = block_stack.back();
            param_declarer_tree->attributes.set(ATTR_param);
            param_declarer_tree->attributes.set(ATTR_variable);
            param_declarer_tree->attributes.set(ATTR_lval);
            param_symbol_tree = new_symbol(param_declarer_tree);
            const string *param_name = param_declid_tree->lexinfo;
            param_check = function_symbol->parameters[i];
            if(param_name != param_check->para_type){
               errllocprintf(base_type_tree->lloc, 
                        "error: \'%s\' wrong \n", 
                        param_name->c_str());
               return errors + 1;
            }
            if(!check_same_attr(param_declarer_tree->attributes, 
                        param_check->attributes)){
               errllocprintf (param_declarer_tree->children[0]->lloc, 
                        "error: \'%s\' wrong \n", 
                        param_name->c_str());
               return errors + 1;
            }
            if(errors == 0){
               insert_identif(param_declid_tree->lexinfo,
                              param_symbol_tree);
               function_symbol->parameters.push_back(
                              param_symbol_tree);
               print_sym(param_declid_tree->lexinfo,
                     param_symbol_tree->lloc,
                     param_symbol_tree->symbol_attr_string(),
                     param_symbol_tree->block_nr);
            }
         }
      }
      errors += typecheck_assistor(block);
   }
   return_attr = ATTR_void;
   pop_block();
   current_indent--;
   return errors;
}

//------ type check---------
int typecheck_assistor(astree *node){
   int errors = 0;
   astree *left_child = nullptr;
   astree *right_child = nullptr;
   if(node->children.size() == 2){
      left_child = node->children[0];
      right_child = node->children[1];
   }else if(node->children.size() == 1){
      left_child = node->children[0];
   }
   switch(node->symbol){
      case TOK_STRUCT:{
         errors += process_struct(node);
         break;
      }
      case TOK_ROOT:{
         for(astree* child: node->children){
            typecheck_assistor(child);
         }
         node->block_nr = 0;
         break;
      }
      case TOK_NULL:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_null);
         node->attributes.set(ATTR_const);
         break;
      }
      case '=':{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         errors += typecheck_assistor(right_child);
         if(!left_child->attributes.test(ATTR_lval)){
            errllocprintf (left_child->lloc, 
            "error: type %s is wrong\n",
            left_child->treeTypeFunc().c_str());
         }
         if(check_astree_compatible(left_child, right_child)){
            inher_type(node, left_child);
            node->attributes.set(ATTR_vreg);
         }else{
            errllocprintf(node->lloc, 
            "error: type %s is wrong\n", 
            right_child->treeTypeFunc().c_str());
            errors += 1;
         }
         break;
      }
      case '+': 
      case '-': 
      case '*': 
      case '/': 
      case '%':{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         errors += typecheck_assistor(right_child);
         if(check_int_type(left_child) && check_int_type(right_child)){
            node->attributes.set(ATTR_int);
            node->attributes.set(ATTR_vreg);
         }else{
            errllocprintf(node->lloc, 
            "error: \'%s\' is wrong\n" ,
            node->lexinfo->c_str());
            errors++;
         }
         break;
      }
      case TOK_INDEX:{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         errors += typecheck_assistor(right_child);
         if(!right_child->attributes.test(ATTR_int)){
            errllocprintf(right_child->lloc, 
            "error: %s is invalid\n",
            right_child->astree_string_attr().c_str());
            errors++;
         }
         if(left_child->attributes.test(ATTR_array)){
            inher_type(node, left_child);
            node->attributes.reset(ATTR_array);
            node->attributes.set(ATTR_vaddr);
            node->attributes.set(ATTR_lval);
         }else if(left_child->attributes.test(ATTR_string)){
            node->attributes.set(ATTR_int);
            node->attributes.set(ATTR_vaddr);
            node->attributes.set(ATTR_lval);
         }else{
            errllocprintf (right_child->lloc, 
            "error: type %s is wrong\n",
            left_child->treeTypeFunc().c_str());
            errors++;
         }
         break;
      }
      case TOK_IDENT:{
         node->block_nr = block_stack.back();
         symbol *ident_symbol = find_identer(node);
         if(ident_symbol == nullptr){
            errllocprintf(node->lloc, 
            "error: \'%s\' is in wrong scope\n", 
                  node->lexinfo->c_str());
            return errors + 1;
         }
         inher_attributes(node,ident_symbol);
         node->definedl = ident_symbol->lloc;
         break;
      }
      case TOK_NEWSTRING:{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         if(check_int_type(left_child)){
            node->attributes.set(ATTR_string);
            node->attributes.set(ATTR_vreg);
         }else{
            errllocprintf (left_child->lloc, 
            "error: not type int\n", 
            left_child->lexinfo->c_str());
            return errors + 1;
         }
         break;
      }
      case TOK_FUNCTION:{
         errors += process_function(node);
         break;
      }
      case TOK_RETURNVOID:{
         node->block_nr = block_stack.back();
         if(return_attr != ATTR_void){
            errllocprintf (node->lloc, 
            "error: return is invalid \'%s\'\n", 
            node->lexinfo->c_str());
            errors++;
         }
         break;
      }
      case TOK_FIELD:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_field);
         break;
      }
      case TOK_RETURN:{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         if(!left_child->attributes.test(return_attr)){
            errllocprintf(node->lloc, 
               "error: return is invalid \'%s\'\n", 
               node->lexinfo->c_str());
            errors++;
         }else if(return_attr == ATTR_typeid &&
                 left_child->treeType != return_struct){
            errllocprintf (node->lloc, 
            "error: return is invalid \'%s\'\n", 
            node->lexinfo->c_str());
         }else{
            returned = 1;
         }
         break;
      }
      case TOK_INTCON:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_int);
         node->attributes.set(ATTR_const);
         break;
      }
      case TOK_WHILE:
      case TOK_IF:
      case TOK_IFELSE:{
         node->block_nr = block_stack.back();
         for(size_t i = 0; i < node->children.size(); i++){
            errors += typecheck_assistor(node->children[i]);
         }
         break;
      }
      case TOK_BLOCK:{
         push_block();
         current_indent++;
         node->block_nr = block_stack.back();
         for(size_t i = 0; i < node->children.size(); i++){
            errors += typecheck_assistor(node->children[i]);
         }
         pop_block();
         current_indent--;
         break;
      }
      case TOK_VARDECL:{
         errors += process_vardecl(node);
         break;
      }
      case TOK_CALL:{
         node->block_nr = block_stack.back();
         symbol *function_symbol = find_identer(node->children[0]);
         errors += typecheck_assistor(node->children[0]);
         if(function_symbol == nullptr){
            errllocprintf (node->lloc, 
                     "error: \'%s\'  can not find\n", 
                     node->children[0]->lexinfo->c_str());
            return errors + 1;
         }
         if(!function_symbol->attributes.test(ATTR_function)){
            errllocprintf (node->lloc, 
                     "error: \'%s\' not function\n", 
                     node->children[0]->lexinfo->c_str());
            return errors + 1;
         }
         astree *parameters_tree = node;
         if(parameters_tree->children.size() - 1 != 
            function_symbol->parameters.size()){
            errllocprintf(node->lloc, 
               "error: wrong arguments\n", "");
            return errors + 1;
         }
         astree *param;
         for(size_t i = 1; i < parameters_tree->children.size(); i++){
            param = parameters_tree->children[i];
            errors += typecheck_assistor(param);
            if(!check_attr_compatible(param->attributes, 
               function_symbol->parameters[i - 1]->attributes)){
               errllocprintf (node->lloc, 
               "error:  argument wrong\n", "");
               return errors + 1;
            }
         }
         inher_type(node, function_symbol);
         node->attributes.set(ATTR_vreg);
         break;
      }
      case TOK_NEWARRAY: {
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         errors += typecheck_assistor(right_child);
         if(check_int_type(right_child)){
            inher_type(node, left_child);
            node->attributes.set(ATTR_vreg);
            node->attributes.set(ATTR_array);
         }else{
            errllocprintf (left_child->lloc, 
               "error: not type int\n", 
               left_child->lexinfo->c_str());
            return errors + 1;
         }
         break;
      }
      case TOK_NEW:{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         inher_type(node, left_child);
         node->attributes.set(ATTR_vreg);
         break;
      }
      case '.':{
      node->block_nr = block_stack.back();
      errors += typecheck_assistor(left_child);
      errors += typecheck_assistor(right_child);
      if(!left_child->attributes.test(ATTR_struct)){
         errllocprintf(left_child->lloc, 
         "error: \'%s\' not struct\n",
         left_child->lexinfo->c_str());
         return errors + 1;
      }
      auto find_struct = struct_table->find(left_child->treeType);
      if(find_struct == struct_table->end()){
         errllocprintf (left_child->lloc, 
         "error: struct \'%s\' declare wrong scope\n", 
         left_child->treeType->c_str());
         return errors + 1;
      }
      if(find_struct->second == nullptr){
         errllocprintf (left_child->lloc, 
         "error: type wrong \'%s\'\n",
         left_child->treeType->c_str());
         return errors + 1;
      }
      symbol_table *fields = find_struct->second->fields;
      auto find_field = fields->find(right_child->lexinfo);
      if(find_field == fields->end()){
         errllocprintf(right_child->lloc, 
         "error: \'%s\' declare not in struct\n",
         right_child->lexinfo->c_str());
         return errors + 1;
      }
      inher_attributes(node, find_field->second);
      node->attributes.set(ATTR_vaddr);
      node->attributes.reset(ATTR_field);
      node->definedl = find_field->second->lloc;
      node->attributes.set(ATTR_lval);
      break;
      }
      case TOK_CHARCON:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_int);
         node->attributes.set(ATTR_const);
         break;
      }
      case TOK_EQ: 
      case TOK_NE: 
      case TOK_LT: 
      case TOK_LE: 
      case TOK_GT: 
      case TOK_GE:{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         errors += typecheck_assistor(right_child);
         if(check_astree_compatible(left_child, right_child)){
            node->attributes.set(ATTR_int);
            node->attributes.set(ATTR_vreg);
         }else{
            errllocprintf (node->lloc, 
            "error: \'%s\' invalid typen\n" ,
            node->lexinfo->c_str());
            errors++;
         }
         break;
      }
      case TOK_STRINGCON:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_string);
         node->attributes.set(ATTR_const);
         break;
      }
      case TOK_POS:
      case TOK_NEG: 
      case '!':{
         node->block_nr = block_stack.back();
         errors += typecheck_assistor(left_child);
         if(check_int_type(left_child)){
            node->attributes.set(ATTR_int);
            node->attributes.set(ATTR_vreg);
         }else{
            errllocprintf(node->lloc, 
            "error: \'%s\' invalid type\n" ,
            node->lexinfo->c_str());
            errors++;
         }
         break;
      }
      case TOK_VOID:{
         node->block_nr = block_stack.back();
         errllocprintf (node->lloc, 
         "error: void wrong\n",
         node->lexinfo->c_str());
         errors++;
         node->attributes.set(ATTR_void);
         break;
      }
      case TOK_INT:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_int);
         break;
      }
      case TOK_STRING:{
         node->block_nr = block_stack.back();
         node->attributes.set(ATTR_string);
         break;
      }
      case TOK_TYPEID:{
         node->block_nr = block_stack.back();
         auto find_struct = struct_table->find(node->lexinfo);
         if(find_struct == struct_table->end()){
            errllocprintf (node->lloc, 
            "error: struct \'%s\' wrong declare\n", 
            node->lexinfo->c_str());
            errors++;
         }else if(find_struct->second == nullptr){
            errllocprintf(node->lloc, 
            "error: type wrong \'%s\'\n",
            node->lexinfo->c_str());
            errors++;
         }
         node->attributes.set(ATTR_struct);
         node->treeType = node->lexinfo;
         break;
      }
   }
   return errors;
}


int typecheck(astree *tree){
   symbol_stack.push_back(new symbol_table);
   block_stack.push_back(0);
   struct_table = new symbol_table;
   return typecheck_assistor(tree);
}

