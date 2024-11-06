#ifndef _LEXER_H_
#define _LEXER_H_

#define MAX_STRING_LENGTH (2048)
#define MAX_LIST_DEPTH (5)
#define NO_TYPES (7)
// Uncomment this for verbose lectical analysis
// #define VERBOSE_LEXER

void yyerror(const char *s);

#endif
