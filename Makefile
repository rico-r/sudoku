CFLAGS=-Ofast -Wall

build: sudoku
	@echo Done. Now run \"./sudoku\".

sudoku: main.o sudoku.o
	$(CC) -pie -o sudoku main.o sudoku.o
	strip -s sudoku

main.o: main.c sudoku.h
	$(CC) $(CFLAGS) -c main.c

sudoku.o: sudoku.c sudoku.h
	$(CC) $(CFLAGS) -c sudoku.c

clean:
	$(RM) sudoku.o main.o sudoku

.PHONY: build clean