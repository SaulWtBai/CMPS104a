// $Id: main.cpp,v 1.2 2016-08-18 15:13:48-07 - - $

#include <string>
#include <vector>
using namespace std;

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <libgen.h>
#include <errno.h>


#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "string_set.h"

constexpr size_t LINESIZE = 1024;
FILE* tokenprint;

//read the file and insert into the ADT
void cppLines(FILE* pipe, char* fileName){
   int linenr = 1;
   char inputName[LINESIZE];
   strcpy(inputName, fileName);
   for(;;){
      char buffer[LINESIZE];
      char* fgets_rc = fgets(buffer, LINESIZE, pipe);
      if(fgets_rc == NULL) break;
      size_t leng = strlen(buffer);
     if(leng != 0){
        char* endpos = buffer + leng -1;
        if(*endpos == '\n') *endpos = '\0';
     }
      int sscanf_rc = sscanf(buffer, 
         "# %d \"%[^\"]\"", &linenr, fileName);
      if(sscanf_rc==2){
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for(int tokenct = 1;; ++tokenct){
         char* token = strtok_r(bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if(token == NULL) break;
         string_set::intern(token);
      }
      ++linenr;
   }
}

void fill(){
   unsigned tokenType;
   while((tokenType = yylex())){
      if(tokenType == YYEOF) break;
   }
}

int main(int argc, char** argv){
   exec::execname = basename (argv[0]);
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
      tokenprint = fopen(tok_file_name.c_str(), "w"); 
      string cppPath = "/usr/bin/cpp";
      string command = cppPath +" " + file_name;
      if(cppOption!=NULL) command = command + "-D" + cppOption;
      
      // call the string set ADT opration to dump the
      // string set into its trace file
      yyin = popen(command.c_str(), "r");
      if(yyin == NULL){
         fprintf(stderr, 
         "File name %s failed to open and can be processed",
         file_name);
         exit(1);
      }else{
         fill();
         cppLines(yyin, file_name);
         // dump the string set into its trace file
         FILE* outfiles = fopen(out_file_name.c_str(), "w");
         string_set::dump(outfiles);
         fclose(outfiles);
         fclose(tokenprint);
         // close the I/O
         int return_pclose = pclose(yyin);
         if(return_pclose!=0){
            exit_status = 1;
         }
      }
   }
   return exit_status;
}

