CFLAGS=-g -Wall
#CFLAGS=-O3

all : src/function.o src/lexer.o src/earing.o src/naming.o src/directives.o src/module.o src/util.o src/error.o src/repl.o src/gen.o src/token.o src/label.o src/dis.o src/strndup.o
	gcc -g -Wall -ldl -o build/earing src/function.o src/lexer.o src/earing.o src/naming.o src/directives.o src/module.o src/util.o src/error.o src/repl.o src/gen.o src/token.o src/label.o src/dis.o /usr/local/lib/libudis86.a src/strndup.o

