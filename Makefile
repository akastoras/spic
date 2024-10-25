CC = gcc
LEXER = lexer.l
PARSER = parser.y
BIN = build/spice.bin
OBJFILES = build/parser.tab.o build/lex.yy.o
CFLAGS = -Wall -g
LFLAGS = #-lm -lfl

all: $(BIN)

build/%.tab.o: %.tab.c %.tab.h
	gcc $(CFLAGS) -o $@ -c $< $(LFLAGS)

build/lex.yy.o: lex.yy.c
	gcc $(CFLAGS) -o $@ -c $< $(LFLAGS)

build/%.o: lib/%.c lib/%.h
	gcc $(CFLAGS) -o $@ -c $< $(LFLAGS)

lex.yy.c: $(LEXER) constants.h
	flex $<

parser.tab.c parser.tab.h: $(PARSER) constants.h
	bison -vd $<

#-Wno-other -Wcounterexamples

$(BIN): $(OBJFILES)
	gcc $(CFLAGS) $^ -o $@ $(LFLAGS)
	

run: all
	./$(BIN) tests/test1.f

clean:
	-@rm build/* parser.output
# -@rm *.tab.* *.yy.c