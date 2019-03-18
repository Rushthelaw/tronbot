CC = gcc
CFLAGS = -Wall -I .
HEADERS = Game.h Player.h GameTree.h lfsr113.h
DEPS = Game.c Player.c GameTree.c lfsr113.c
OUTPUT = build/Game

all: $(DEPS) $(HEADERS)
	$(CC) $(FLAGS) main.c $(DEPS) -o $(OUTPUT)

exec: $(DEPS) $(HEADERS)
	$(CC) $(FLAGS) main.c $(DEPS) -o $(OUTPUT)
	./$(OUTPUT)

testmain: $(DEPS) $(HEADERS)
	$(CC) $(FLAGS) -g main.c $(DEPS) -o $(OUTPUT)
	valgrind --log-file=valgrind.txt ./Game

tests: $(DEPS) $(HEADERS) Tests.c
	$(CC) $(FLAGS) Tests.c $(DEPS) -o build/Tests

maketests: $(DEPS) $(HEADERS) Tests.c
	$(CC) $(FLAGS) -g Tests.c $(DEPS) -o build/Tests
	valgrind --log-file=valgrind.txt build/Tests
