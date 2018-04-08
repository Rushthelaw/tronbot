CC = gcc
CFLAGS = -Wall -I .
HEADERS = Game.h Player.h GameTree.h
DEPS = Game.c Player.c GameTree.c
OUTPUT = Game

all: $(DEPS) $(HEADERS)
	$(CC) $(FLAGS) main.c $(DEPS) -o $(OUTPUT)

exec: $(DEPS) $(HEADERS)
	$(CC) $(FLAGS) main.c $(DEPS) -o $(OUTPUT)
	./$(OUTPUT)

tests: $(DEPS) $(HEADERS) Tests.c
	$(CC) $(FLAGS) Tests.c $(DEPS) -o Tests

maketests: $(DEPS) $(HEADERS) Tests.c
	$(CC) $(FLAGS) -g Tests.c $(DEPS) -o Tests
	valgrind --log-file=valgrind.txt ./Tests
