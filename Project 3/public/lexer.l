%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cstring>
    #include "ast.hpp"
    #include "parser.hpp"
    #include <string>

    void yyerror(const char *); 
%}

/** WRITE ME:
 *  Put any definitions you need here. Add your own regular expressions
 *  below for ALL tokens defined in the manual. You can choose not to use
 *  definitions here.
 */

/* HERE */

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

true {  yylval.u_base_int = 1; 
                return T_BOOL_LITERAL; }
false {  yylval.u_base_int = 0; 
                return T_BOOL_LITERAL; }
\'[^\"\n]\'         {   yylval.u_base_int = yytext[1];
                        return T_CHAR_LITERAL;}

[0]|[1-9][0-9]* {yylval.u_base_int = std::atoi(yytext); 
                 return T_INTEGER_LITERAL; }

[0][X|x][0-9a-fA-F]* {yylval.u_base_int = std::stoi(yytext+2, nullptr, 16); 
                      return T_INTEGER_LITERAL; 
                      }

[0][0-7]* {yylval.u_base_int = std::stoi(yytext+1, nullptr, 8); 
           return T_INTEGER_LITERAL; }
                        
[01]+b {char* rawNum = strdup(yytext); 
          rawNum[strlen(rawNum)-1] = '\0'; 
          yylval.u_base_int = std::stoi(rawNum, nullptr, 2); 
          return T_INTEGER_LITERAL; }

\"[^\"\n]*\"        {   yylval.u_base_charptr = strdup(yytext);
                        yylval.u_base_charptr = strdup(yytext + 1); 
                        yylval.u_base_charptr[strlen(yylval.u_base_charptr) - 1] = '\0';
                        return T_STRING_LITERAL; }
[a-zA-Z][0-9_a-zA-Z]*       {   yylval.u_base_charptr = strdup(yytext); 
                                return T_IDENTIFIER_LITERAL; }

[ \t\n]             ; /* skip whitespace */
\/%(%[^\/]|[^%]|\n)*%\/     ;



%%

/** You shall not pass!
 *  You should not  have to do or edit anything past this.
 */

int yywrap(void) {
    return 1;
}
