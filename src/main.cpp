#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "lexer.h"

int main(int argc, char **argv)
{
	int token;

	if (argc > 1) {
		yyin = fopen(argv[1], "r");
		if (yyin == NULL) {
			perror("Error opening file");
			return -1;
		}
	}
	
	printf("Start of lexer\n");
	do {
		token = yylex();
	} while (token != 0);

	fclose(yyin);
	// yyterminate();
}
