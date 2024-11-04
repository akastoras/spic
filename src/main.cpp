#include <iostream>
#include <unordered_map>

#include "parser.h"
#include "lexer.h"

#include "node_table.h"


int main(int argc, char **argv)
{
	// Get input file.
	if (argc > 1) {
		yyin = fopen(argv[1], "r");
		if (yyin == NULL) {
			char buff[20];
			sprintf(buff, "Error opening file %s", argv[1]);
			perror(buff);
			return -1;
		}
	}
	
	// This is where the magic happens
	yyparse();

	// Free memory & Cleanup
	fclose(yyin);

	return 0;
}
