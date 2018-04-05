%{
#include <assert.h>
#include <cassert>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%printer { astree::dump (yyoutput, $$); } <>

%initial-action {
   parser::root = new astree (TOK_ROOT, {0, 0, 0}, "");
}


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
%left '[' '.' TOK_CALL
%nonassoc '('

%start start

%%

start       : program             { $$ = $1; }
            ;

program     : program structdef   { $$ = $1->adopt ($2); }
            | program function    { $$ = $1->adopt ($2); }
            | program statement   { $$ = $1->adopt ($2); }
            | program error '}'   { free_ast($3); $$ = $1; }
            | program error ';'   { free_ast($3); $$ = $1; }
            |                     { $$ = parser::root; }           
            ;

structdef   : morefie '}' { free_ast($2); $$ = $1; }
            | TOK_STRUCT TOK_IDENT '{' '}'        
                 { $2->symbol = TOK_TYPEID;$$ = $1->adopt($2); 
                 free_ast($3, $4); }
            ;

morefie     : morefie fielddecl ';' {free_ast($3); $$=$1->adopt($2);}
            | TOK_STRUCT TOK_IDENT '{' fielddecl ';'
                  { $2->symbol = TOK_TYPEID;free_ast($3, $5); 
                    $$ = $1->adopt($2, $4); }
            ;

fielddecl   : basetype TOK_ARRAY TOK_IDENT               
                  { $3->symbol = TOK_FIELD; $$ = $2->adopt($1, $3);}
            | basetype TOK_IDENT 
               {$2->symbol=TOK_FIELD;$$= $1->adopt($2); }
            ;

function    : identdecl morefun ')' block 
               { $2->symbol = TOK_PARAMLIST; free_ast($3); 
                 $$ = adoptFunction($1, $2, $4); }
            | identdecl '(' ')' block   
               { $2->symbol = TOK_PARAMLIST; free_ast($3); 
                 $$ = adoptFunction($1, $2, $4); }
            ;

morefun    : '(' identdecl { $$ = $1->adopt($2);}
            | morefun ',' identdecl    
                  { free_ast($2); $$ = $1->adopt($3);}
            ;      
            
basetype    : TOK_VOID  { $$ = $1; }
            | TOK_IDENT { $1->symbol = TOK_TYPEID; $$ = $1; }
            | TOK_STRING   { $$ = $1; }
            | TOK_INT   { $$ = $1; }
            ;
                  
                  
identdecl   : basetype TOK_IDENT        
            { $2->symbol = TOK_DECLID; $$ = $1->adopt($2); } 
            | basetype TOK_ARRAY TOK_IDENT 
              { $3->symbol = TOK_DECLID;$$ = $2->adopt($1, $3); }
            ;

block       : '{' '}' { free_ast($2); $1->symbol=TOK_BLOCK;$$ = $1;}
            | statements '}' { free_ast($2); 
               $1->symbol = TOK_BLOCK; $$ = $1; }
            |';' { $1->symbol = TOK_BLOCK; $$ = $1; }
            ;
            
statement   : vardecl   { $$ = $1; }
            | return { $$ = $1; }
            | block  { $$ = $1; }
            | while  { $$ = $1; }
            | ifelse { $$ = $1; }
            | expr ';'  { $$ = $1; free_ast($2); }
            ;



statements  : '{' statement   { $$ = $1->adopt($2); }
            | statements statement  { $$ = $1->adopt($2); }
            ;

vardecl     : identdecl '=' expr ';'    
               { $2->symbol = TOK_VARDECL;
               $$ = $2->adopt($1, $3); free_ast($4); }
            ;


while       : TOK_WHILE  '(' expr ')' statement {
               $$ = $1->adopt($3, $5); free_ast($2, $4);}
            ;

ifelse      : if statement TOK_ELSE statement  
               { $1->symbol = TOK_IFELSE;
                 $$ = $1->adopt($2, $4); free_ast($3); }
            | if statement %prec TOK_ELSE  
               { $$ = $1->adopt($2); }

if          : TOK_IF '(' expr ')'{
              $$ = $1->adopt($3); free_ast($2, $4);
              }
            ;   
  
return      : TOK_RETURN ';' {
               $1->symbol = TOK_RETURNVOID; $$ = $1; free_ast($2);
            }
            | TOK_RETURN expr ';'{
              $$ = $1->adopt($2); free_ast($3); 
            }
            ;

expr        : binop  {$$= $1;}
            | unop   {$$=$1;}
            | call   { $$ = $1; }
            | allocator { $$ = $1; }
            | '(' expr ')' { $$ = $2; free_ast($1, $3); }
            | variable  { $$ = $1; } 
            | constant  { $$ = $1; }
            ;
   
binop       : expr '+'    expr          { $$ = $2->adopt($1, $3); }
            | expr '-'    expr          { $$ = $2->adopt($1, $3); }
            | expr '*'    expr          { $$ = $2->adopt($1, $3); }
            | expr '/'    expr          { $$ = $2->adopt($1, $3); }
            | expr '%'    expr          { $$ = $2->adopt($1, $3); }   
            | expr '='    expr          { $$ = $2->adopt($1, $3); }
            | expr TOK_EQ expr          { $$ = $2->adopt($1, $3); }
            | expr TOK_NE expr          { $$ = $2->adopt($1, $3); }
            | expr TOK_LT expr          { $$ = $2->adopt($1, $3); }
            | expr TOK_LE expr          { $$ = $2->adopt($1, $3); }
            | expr TOK_GT expr          { $$ = $2->adopt($1, $3); }
            | expr TOK_GE expr          { $$ = $2->adopt($1, $3); }
            ;
            
unop        : '+' expr  { $1->symbol = TOK_POS; $$ = $1->adopt($2); }
            | '-' expr  { $1->symbol = TOK_NEG; $$ = $1->adopt($2); }
            | '!' expr  { $$ = $1->adopt($2); }
            ;

allocator   :TOK_NEW TOK_STRING '(' expr ')' {
             $1->symbol = TOK_NEWSTRING; free_ast($2, $3); 
             free_ast($5); $$ = $1->adopt($4); }   
 
            | TOK_NEW basetype '[' expr ']' {
              $1->symbol = TOK_NEWARRAY; $$ = $1->adopt($2, $4);
               free_ast($3, $5); 
            }                              
            |TOK_NEW TOK_IDENT '(' ')' {
             $2->symbol = TOK_TYPEID; free_ast($3, $4); 
             $$ = $1->adopt($2); 
            };
           
                                          
  
call        : TOK_IDENT '(' ')' { 
              free_ast($3); $2->symbol=TOK_CALL; $$=$2->adopt($1);
            }
            | TOK_IDENT argcall ')'{ 
              free_ast($3); $2->symbol=TOK_CALL;
              $$ = $2->forwardAdopt($1); 
            }

argcall    : '(' expr  { $$ = $1->adopt($2); }
            | argcall ',' expr  { 
               free_ast($2);  $$ = $1->adopt($3); 
            };                              

constant   : TOK_INTCON {$$ = $1;}| TOK_STRINGCON{$$=$1;}
            | TOK_CHARCON { $$ = $1; }| TOK_NULL{ $$ = $1; };

            
variable    : TOK_IDENT  { $$ = $1; }
            | expr '[' expr ']' { $2->symbol = TOK_INDEX;
               $$ = $2->adopt($1, $3); free_ast($4); }
            | expr '.' TOK_IDENT {
                $3->symbol = TOK_FIELD; $$ = $2->adopt($1, $3);
            }
            ;            

%%

const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

