
%{
	#include <stdio.h>
	int yylex(void);
	void yyerror(char *);
%}


%%

List    : List Expr '.' {printf("parsed expresion\n");}
	| Expr '.' {printf("parsed expresion\n");}
        ;

Expr    : Expr '+' Expr 
        | Expr '-' Expr 
        | Expr '*' Expr
        | 'n'
        | '-' Expr  
        | '(' Expr ')'
        | '|' Expr '|'
        ;

%%

void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
	return;
}

int main(void) {
	yyparse();
	return 0;
}
