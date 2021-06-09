CC=gcc

CFLAGS=-Wall -std=c99

OBJS= token.o lex.yy.o util.o parser.o generator.o

yowaic: yowaic.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ yowaic.o $(OBJS)

yowaic.o: yowaic.c
	$(CC) -c yowaic.c

util.o: util.c
	$(CC) -c util.c

token.o: token.c
	$(CC) -c token.c

parser.o: parser.c
	$(CC) -c parser.c

generator.o: generator.c
	$(CC) -c generator.c

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: scanner.l
	flex scanner.l

unit_test.o: unit_test.c
	$(CC) -c unit_test.c

unit_test: unit_test.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ unit_test.o $(OBJS)

test: unit_test
	./unit_test
	./test.sh

clean:
	rm yowaic unit_test *.o foo.*
