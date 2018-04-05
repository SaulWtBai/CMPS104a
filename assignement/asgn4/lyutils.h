#ifndef __UTILS_H__
#define __UTILS_H__

// Lex and Yacc interface utility.

#include <string>
#include <vector>
using namespace std;

#include <stdio.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "string_set.h"

extern FILE* yyin;
extern char* yytext; 
extern int yy_flex_debug;
extern int yydebug;
extern size_t yyleng;
extern FILE* tokenprint;
extern FILE* astprint;
extern FILE* symprint;

extern string_set *string_dump;

#define YYEOF 0

int yylex();
int yylex_destroy();
int yyparse();
void yyerror (const char* message);
astree *adoptFunction(astree* tree1, astree* tree2, astree* tree3);

struct lexer {
   static bool interactive;
   static location lloc;
   static size_t last_yyleng;
   static vector<string> filenames;
   static const string* filename (int filenr);
   static void newfilename (const string& filename);
   static void advance();
   static void newline();
   static void badchar (unsigned char bad);
   static void badtoken (char* lexeme);
   static void include();
   static int yylval_token(int symbol);
   static void dumpToken(astree* dump, FILE* DUMP);
};

struct parser {
   static astree* root;
   static const char* get_tname (int symbol);
};

#define YYSTYPE_IS_DECLARED
typedef astree* YYSTYPE;
#include "yyparse.h"

#endif

