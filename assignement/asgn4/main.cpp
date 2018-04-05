#include <string>
using namespace std;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utility>
#include <errno.h>
#include <libgen.h>
#include <wait.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "string_set.h"
#include "symtable.h"


constexpr size_t LINESIZE = 1024;
FILE* outfiles;
//FILE* tokenprint;
//FILE* astprint;
//FILE* symprint;

int main(int argc, char** argv){
   exec::execname = basename(argv[0]);
   
   // record the status when program exits
   int exit_status = 0;
   // record the option for execute cpp file
   char *cppOption = NULL;
   int option;
   yy_flex_debug = 0;
   yydebug = 0;
   
   // judgement for SYNOPSIS
   while((option = getopt(argc, argv, "@:D:ly"))!=1){
      if(option == EOF) break;
      switch(option){
         case 'l':
            yy_flex_debug = 1;
            break;
         case 'y':
            yydebug = 1;
            break;
         case '@':
            break;
         case 'D':
            cppOption = optarg;
            break;
      }
   }
   
   //check If the index of getopt() is over bound
   if (optind >= argc) {
      fprintf(stderr, "Unexpected argument\n");
      exit(1);
   }
   
   // Check and change filename
   for(int i = optind; i<argc; i++){
      char* file_name = argv[i];
      //remove all extension ".oc"
      string base_name;
      string str_file_name = string(file_name);
      size_t extension_dot = str_file_name.find_last_of(".");
      
      if( str_file_name.substr(
         extension_dot+1, std::string::npos
      ).std::string::compare("oc")!=0 ){
         fprintf(stderr, "%s is not .oc file \n", file_name);
         exit(1);
      }
      if( extension_dot == std::string::npos ){
         base_name = str_file_name;
      }else{
         base_name = str_file_name.substr(0, extension_dot);
      }
      // change all extension to ".str" for output file
      string out_file_name = base_name + ".str";
      string tok_file_name = base_name + ".tok";
      string ast_file_name = base_name + ".ast";
      string sym_file_name = base_name + ".sym";
      
      //open file
      outfiles = fopen(out_file_name.c_str(), "w");
      tokenprint = fopen(tok_file_name.c_str(), "w");
      astprint = fopen(ast_file_name.c_str(), "w");
      symprint = fopen(sym_file_name.c_str(), "w");
      
      string cppPath = "/usr/bin/cpp";
      string command = cppPath +" " + file_name;
      if(cppOption!=NULL) command = command + "-D" + cppOption;
      
      
      yyin = popen (command.c_str(), "r");
      if(yyin == NULL) {
         fprintf(stderr, 
         "File name %s failed to open and can be processed",
         file_name);
         exit(1);
      }else{
         if (yy_flex_debug) {
            fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",
                  command.c_str(), fileno (yyin));
         }
         lexer::newfilename (command);
      }
      
     
      string_dump = new string_set();
      int parsecode = 0;

      parsecode = yyparse();
      if(parsecode){
         errprintf ("%:parse failed (%d)\n", parsecode);
      }

      typecheck(parser::root);
      astree::print(astprint, parser::root, 0);
      string_dump->dump(outfiles);
  
      free_ast(parser::root);
      delete string_dump;

      fclose(outfiles);
      fclose(tokenprint);
      fclose(astprint);
      fclose(symprint);
      return exit_status;
   }
   
}
