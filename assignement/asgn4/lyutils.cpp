
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlib.h"
#include "lyutils.h"

bool lexer::interactive = false;
location lexer::lloc = {0, 1, 0};
size_t lexer::last_yyleng = 0;
vector<string> lexer::filenames;
FILE* tokenprint;
FILE* astprint;
FILE* symprint;

string_set *string_dump;

astree* parser::root = nullptr;


const string* lexer::filename (int filenr) {
   return &lexer::filenames.at(filenr);
}

void lexer::newfilename (const string& filename) {
   lexer::lloc.filenr = lexer::filenames.size();
   lexer::filenames.push_back (filename);
}

void lexer::advance() {
   if (not interactive) {
      if (yy_flex_debug) {
        if (lexer::lloc.offset == 0) {
          fprintf (stderr, ";%2zd.%3zd: ",
            lexer::lloc.filenr, lexer::lloc.linenr);
        }
        fprintf (stderr, "%s", yytext);

      }
   }
   lexer::lloc.offset += last_yyleng;
   last_yyleng = yyleng;
}

void lexer::newline() {
   ++lexer::lloc.linenr;
   lexer::lloc.offset = 0;
}

void lexer::badchar (unsigned char bad) {
   char buffer[16];
   snprintf (buffer, sizeof buffer,
             isgraph (bad) ? "%c" : "\\%03o", bad);
   errllocprintf (lexer::lloc, "invalid source character (%s)\n",
                  buffer);
}

void lexer::badtoken (char* lexeme) {
   errllocprintf (lexer::lloc, "invalid token (%s)\n", lexeme);
}

//----hw2
void lexer::include() {
   size_t linenr;
   static char filename[0x1000];
   assert (sizeof filename > strlen (yytext));
   int scan_rc = sscanf (yytext,"# %zd \"%[^\"]\"",&linenr, filename);
   if (scan_rc != 2) {
      errprintf ("%s: invalid directive, ignored\n", yytext);
   }else {
      fprintf(tokenprint, "# %zd \"%s\"\n", linenr, filename);
      if (yy_flex_debug) {
         fprintf (stderr, "--included # %zd \"%s\"\n",
                  linenr, filename);
      }
      lexer::lloc.linenr = linenr - 1;
      lexer::newfilename (filename);
   }
}

int lexer::yylval_token(int symbol){
   int offset = lexer::lloc.offset - yyleng;
   yylval = new astree(symbol, lexer::lloc, yytext);
   yylval->lloc.offset = offset;
   yylval->lloc.filenr = lexer::filenames.size() - 1;
   yylval->lloc.linenr = lexer::lloc.linenr;
   dumpToken(yylval, tokenprint);
   return symbol;
}

void lexer::dumpToken(astree* dump, FILE* DUMP){
   fprintf(DUMP, "%5zu %2zu.%03zu %4u  %-15s (%s)\n", 
      dump->lloc.filenr,
      dump->lloc.linenr,
      dump->lloc.offset,
      dump->symbol,
      parser::get_tname(dump->symbol),
      dump->lexinfo->c_str());
}


void yyerror (const char* message) {
   assert (not lexer::filenames.empty());
   errllocprintf (lexer::lloc, "%s\n", message);
}

astree *adoptFunction(astree* tree1, astree* tree2, astree* tree3){
   astree* funTree = new astree(TOK_FUNCTION, tree1->lloc, "");
   funTree->adopt(tree1, tree2);
   funTree->adopt(tree3);
   return funTree;
}



