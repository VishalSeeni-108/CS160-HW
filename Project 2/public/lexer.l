%option yylineno
%pointer

%{
    #include <stdlib.h>
    #include <string.h>
    #include "parser.hpp"

    void yyerror(const char *);
%}

/** WRITE ME:
 *  Put any definitions you need here. Add your own regular expressions
 *  below for ALL tokens defined in the manual. You can choose not to use
 *  definitions here.
 */

/** The example below is very simple, but it shows almost everything you will
 *  need to do for this first part. The syntax is as follows:
 *
 *  regular_expression      {  c code  }
 *
 * The lexer will try to match each regular expression. When it does it will
 * execute the corresponding C code.
*/

%%



"boolean"           { return T_BOOLEAN; }
"char"              { return T_CHAR; }
"integer"           { return T_INTEGER; }
"string"            { return T_STRING; }
"intptr"            { return T_INTPTR; }
"charptr"           { return T_CHARPTR; }
"if"                { return T_IF; }
"else"              { return T_ELSE; }
"while"             { return T_WHILE; }
"var"               { return T_VAR; }
"procedure"         { return T_PROCEDURE; }
"return"            { return T_RETURN; }
"null"              { return T_NULL; }

"&&"                { return T_AND; }
"/"                 { return T_DIVIDE; }
"="                 { return T_EQUAL; }
"=="                { return T_BOOL_EQUAL; }
">"                 { return T_GREATER_THAN; }
">="                { return T_GREATER_THAN_OR_EQUAL; }
"<"                 { return T_LESS_THAN; }
"<="                { return T_LESS_THAN_OR_EQUAL; }
"-"                 { return T_MINUS; }
"!"                 { return T_NOT; }
"!="                { return T_NOT_EQUAL; }
"||"                { return T_OR; }
"+"                 { return T_PLUS; }
"*"                 { return T_TIMES; }
"&"                 { return T_REFERENCE; }
"^"                 { return T_DEREFERENCE; }

";"                 { return T_SEMICOLON; }
":"                 { return T_COLON; }
","                 { return T_COMMA; }
"|"                 { return T_BAR; }
"{"                 { return T_OPEN_CURLY; }
"}"                 { return T_CLOSE_CURLY; }
"("                 { return T_OPEN_PARAN; }
")"                 { return T_CLOSE_PARAN; }
"["                 { return T_OPEN_SQUARE; }
"]"                 { return T_CLOSE_SQUARE; }

(true|false) {return T_BOOL_LITERAL; }
\'[^\"\n]\'         {return T_CHAR_LITERAL;}
[1-9][0-9]*|0|0[xX][0-9a-fA-F]+|0[0-7]*|[01]+b   {return T_INTEGER_LITERAL; }
\"[^\"\n]*\"        { return T_STRING_LITERAL; }
[a-zA-Z][0-9_a-zA-Z]*       { return T_IDENTIFIER_LITERAL; }

[ \t\n]             ; /* skip whitespace */
\/%(%[^\/]|[^%]|\n)*%\/     ;

.                   yyerror((char *) "invalid character");

%%

/** You shall not pass!
 *  You should not  have to do or edit anything past this.
 */

int yywrap(void) {
    return 1;
}
