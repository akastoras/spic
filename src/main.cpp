#include <iostream>
#include <unordered_map>

// #include "parser.h"
// #include "lexer.h"

#include "node_table.h"



int main(int argc, char **argv)
{
	spic::NodeTable nt = spic::NodeTable();
	std::string s = std::string("n1");

	nt.append_node(s);
	
	std::cout << nt;

// 	int token;

// 	if (argc > 1) {
// 		yyin = fopen(argv[1], "r");
// 		if (yyin == NULL) {
// 			perror("Error opening file");
// 			return -1;
// 		}
// 	}
	
// 	printf("Start of lexer\n");
// 	do {
// 		token = yylex();
// 	} while (token != 0);

// 	fclose(yyin);
	// yyterminate();
}
