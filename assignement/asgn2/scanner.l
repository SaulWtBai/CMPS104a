/* $Id: lexer.l,v 1.7 2016-10-27 19:45:59-07 - - $ */

%{
#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }
#define IGNORE(THING) {}

%}

%option 8bit
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
%option verbose

LETTER   [A-Za-z_]
DIGIT   [0-9]

MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({MANTISSA}{EXPONENT}?)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
INVALIDENT      ({NUMBER}+{IDENT})

CHAR            ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")
BADSTRING       (\"[^\n\"\\]*(.|\n))
ABADCHAR        ('(.*)|\n')
BADCHAR         ('[\n\'\\]'|'.[^\'])

%%

"#".*           { lexer::include(); }
\n              { lexer::newline(); }
[ \t]+          { IGNORE (white space) }

"if"            { return lexer::yylval_token(TOK_IF); }
"else"          { return lexer::yylval_token(TOK_ELSE); }
"while"         { return lexer::yylval_token(TOK_WHILE); }
"return"        { return lexer::yylval_token(TOK_RETURN); }
"void"          { return lexer::yylval_token(TOK_VOID); }
"char"          { return lexer::yylval_token(TOK_CHAR); }
"int"           { return lexer::yylval_token(TOK_INT); }
"string"        { return lexer::yylval_token(TOK_STRING); }
"struct"        { return lexer::yylval_token(TOK_STRUCT); }
"null"          { return lexer::yylval_token(TOK_NULL); }
"ord"           { return lexer::yylval_token(TOK_ORD); }
"new"           { return lexer::yylval_token(TOK_NEW); }

"+"             { return lexer::yylval_token ('+'); }
"-"             { return lexer::yylval_token ('-'); }
"*"             { return lexer::yylval_token ('*'); }
"/"             { return lexer::yylval_token ('/'); }
"[]"            { return lexer::yylval_token (TOK_ARRAY); }
"=="            { return lexer::yylval_token (TOK_EQ); }
"!="            { return lexer::yylval_token(TOK_NE); }
"<"             { return lexer::yylval_token(TOK_LT); }
"<="            { return lexer::yylval_token(TOK_LE); }
">"             { return lexer::yylval_token(TOK_GT); }
">="            { return lexer::yylval_token(TOK_GE); }
"("             { return lexer::yylval_token('('); }
";"             { return lexer::yylval_token(';'); }
")"             { return lexer::yylval_token(')'); }
"["             { return lexer::yylval_token('['); }
"]"             { return lexer::yylval_token(']'); }
"{"             { return lexer::yylval_token('{'); }
"}"             { return lexer::yylval_token('}'); }
","             { return lexer::yylval_token(','); }
"."             { return lexer::yylval_token('.'); }
"%"             { return lexer::yylval_token('%'); }
"!"             { return lexer::yylval_token('!'); }
"="             { return lexer::yylval_token ('='); }

{IDENT}         { return lexer::yylval_token(TOK_IDENT); }
{CHAR}          { return lexer::yylval_token(TOK_CHARCON); }
{STRING}        { return lexer::yylval_token(TOK_STRINGCON); }
{NUMBER}        { return lexer::yylval_token(TOK_INTCON); }
{INVALIDENT}    { lexer::badtoken(yytext); }
{BADCHAR}       { lexer::badtoken(yytext); }
{BADSTRING}     { lexer::badtoken(yytext); }
.               { lexer::badchar(*yytext); }
%%

