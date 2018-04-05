%{

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "lyutils.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON


%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ROOT


%token TOK_DECLID TOK_INDEX TOK_NEWSTRING TOK_RETURNVOID
%token TOK_VARDECL TOK_FUNCTION TOK_PARAMLIST TOK_PROTOTYPE

%right TOK_IF TOK_ELSE
%right '='
%left TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left '+' '-'
%left '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW
%left TOK_ARRAY TOK_FIELD TOK_FUNCTION
%left '[' '.'
%nonassoc '('

%start start

%%

start      : program    { yyparse_astree = $1; }
           ;

program    : program structdef  { $$ = $1->adopt($2); }
           | program function   { $$ = $1->adopt($2); }
           | program statement  { $$ = $1->adopt($2); }
           | program error '}'  { $$ = $1;}
           | program error ';'  { $$ = $1;}
           |                    { $$ = new_parseroot();}
           ;

headst : '{' fielddecl   { free_ast($1); $$ = $2; }
           | headst fielddecl   {$$ = $1-> adopt($2); }
           ;

structdef  : TOK_STRUCT TOK_IDENT headst '}' {
              free_ast($4); $2 = $2->convert(TOK_TYPEID);
              $$ = $1->adopt($2, $3);
           } 
           |TOK_STRUCT TOK_IDENT '{' '}' {
              free_ast($3, $4); $2 = $2->convert(TOK_TYPEID);
              $$ = $1->adopt($2);
           } 
           ;

fielddecl  : basetype TOK_ARRAY TOK_IDENT ';' {
              free_ast($4); $3 = $3->convert(TOK_FIELD);
              $$ = $2->adopt($1, $3);
           }
           | basetype TOK_IDENT ';' {
              free_ast($3); $2 = $2->convert(TOK_FIELD);
              $$ = $1->adopt($2);
           }
           ;
basetype   : TOK_VOID   {$$ = $1;}
           | TOK_INT   {$$ = $1;}
           | TOK_STRING   {$$ = $1;}
           | TOK_IDENT   {$$ = $1->convert(TOK_TYPEID);}

headf      : '(' identdecl     {$$ = $1->adopt_sym($2, TOK_PARAMLIST);}
           | headf ',' identdecl  {$$ = $1->adopt($3);}
           ;

function   : identdecl headf ')' block {
              free_ast($3);
              astree* newtree = new astree(TOK_FUNCTION, $1->lloc, "");
              $$ = newtree->adopt($1, $2, $4);
           }
           | identdecl '(' ')' block {
              free_ast($3);
              astree* newtree = new astree(TOK_FUNCTION, $1->lloc, "");
              $$ = newtree->adopt($1, $2, $4);
           }
           | identdecl headf ')' ';' {
              free_ast($3, $4);
              astree* newtree = new astree(TOK_FUNCTION, $1->lloc, "");
              $$ = newtree->adopt($1,$2);
           }
           |identdecl '(' ')' ';' {
              free_ast($3, $4);
              astree* newtree = new astree(TOK_FUNCTION, $1->lloc, "");
              $$ = newtree->adopt($1,$2);
           }
           ;

identdecl  : basetype TOK_ARRAY TOK_IDENT {
              $3=$3->convert(TOK_DECLID); $$=$2->adopt($1, $3);
           }
           | basetype TOK_IDENT{ $2=$2->convert(TOK_DECLID);
              $$=$1->adopt($2);
           }
           ;

headb      : headb statement   { $$ = $1->adopt($2);}
           | '{' statement   { $$ = $1->adopt($2);}
           ;       

block      : '{' '}' {free_ast($2); $$ = $1->convert(TOK_BLOCK);}
           | headb '}' {free_ast($2);$$ = $1->convert(TOK_BLOCK);}
           ;

statement  : vardecl { $$ =$1; }
           | return  { $$ = $1; }
           | while  { $$ =$1; }
           | ';' { $$ =$1; }
           | ifelse { $$= $1; }
           | block  {$$ = $1; }
           | expr';'{free_ast($2);}
           ;

vardecl    : identdecl'=' expr';' {free_ast($4);
              $2 = $2->adopt($1,$3);
              $$ =$2->convert(TOK_VARDECL);};

return     : TOK_RETURN expr';'{free_ast($3);
              $$ = $1->adopt($2);}
           | TOK_RETURN';'{free_ast($2);
             $$=$1->convert(TOK_RETURNVOID);}
           ;

while      : TOK_WHILE '(' expr ')' statement {
              free_ast($2, $4);$$ =$1->adopt($3,$5);
           }
           ;

ifelse     : TOK_IF '(' expr ')' statement %prec TOK_IF {
              free_ast($2, $4); $$ = $1->adopt($3, $5);
           }
           | TOK_IF '(' expr ')' statement TOK_ELSE statement{
             free_ast($2, $4, $6);$1=$1->convert(TOK_IFELSE);
             $$ = $1->adopt($3,$5,$7);
           }
           ;

expr       : binop   {$$= $1;}
           | unop   {$$=$1;}
           | call   {$$=$1;}
           | allocator   {$$=$1;}
           | '(' expr ')' {free_ast($1,$3);$$ = $2;}
           | variable {$$=$1;}
           | constant {$$=$1;}
           ;

binop      : expr '+' expr   { $$ = $2->adopt($1, $3);}
           | expr '-' expr   { $$ = $2->adopt($1, $3);}
           | expr '*' expr   { $$ = $2->adopt($1, $3);}
           | expr '/' expr   { $$ = $2->adopt($1, $3);}
           | expr '%' expr   { $$ = $2->adopt($1, $3);}
           | expr '=' expr      { $$ = $2->adopt($1, $3);}
           | expr TOK_EQ expr   { $$ = $2->adopt($1, $3);}
           | expr TOK_NE expr   { $$ = $2->adopt($1, $3);}
           | expr TOK_LT expr   { $$ = $2->adopt($1, $3);}
           | expr TOK_LE expr   { $$ = $2->adopt($1, $3);}
           | expr TOK_GT expr   { $$ = $2->adopt($1, $3);}
           | expr TOK_GE expr   { $$ = $2->adopt($1, $3);}
           ;
  
unop       : '+' expr   { $$ = $1->adopt_sym($2, TOK_POS);}     
           | '-' expr   { $$ = $1->adopt_sym($2, TOK_NEG);}
           | '!' expr   { $$ = $1->adopt($2);}
           ;

allocator  : TOK_NEW TOK_IDENT '(' ')'{
              free_ast($3, $4); $2 = $2->convert(TOK_TYPEID);
              $$ = $1->adopt($2);
           }
           | TOK_NEW TOK_STRING '(' expr ')'{
              free_ast($3, $5); $1 = $1->adopt($4);
              $$ = $1->convert(TOK_NEWSTRING);
           }
           | TOK_NEW basetype '[' expr ']'{
              free_ast($3, $5); $1 = $1->adopt($2, $4);
              $$ = $1->convert (TOK_NEWARRAY);
           }
           ;

morecall   : TOK_IDENT '(' expr {
              $2 = $2->adopt($1, $3);$$ = $2->convert(TOK_CALL);}
           | morecall ',' expr {free_ast($2);$$ = $1->adopt($3);}
           ;

call       : morecall')'{free_ast($2);$$=$1;}
           |TOK_IDENT'(' ')'{free_ast($3);
              $2 = $2->convert(TOK_CALL);$$ = $2->adopt($1);}
           ;
constant   : TOK_INTCON {$$ = $1;}| TOK_STRINGCON{$$=$1;}
            | TOK_CHARCON { $$ = $1; }| TOK_NULL{ $$ = $1; };

variable   : TOK_IDENT   { $$ = $1; }
           | expr '.' TOK_IDENT {
              $3 = $3->convert(TOK_FIELD); 
              $$ = $2->adopt($1,$3);
           }
           |expr '[' expr ']'{free_ast($4);
              $2 = $2->convert(TOK_INDEX);
              $$ = $2->adopt($1,$3);
           }
           ;


%%

const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}
