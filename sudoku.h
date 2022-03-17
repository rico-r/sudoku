#ifndef __SUDOKU_H
#define __SUDOKU_H

typedef enum {
	SudokuNote1 = 1,
	SudokuNote2 = 2,
	SudokuNote3 = 4,
	SudokuNote4 = 8,
	SudokuNote5 = 16,
	SudokuNote6 = 32,
	SudokuNote7 = 64,
	SudokuNote8 = 128,
	SudokuNote9 = 256,
	SudokuNoteAll = 511,
	SudokuNoteOk = 512,
	SudokuNoteFixed = 1024
} SudokuNote;

typedef enum {
	SudokuResultError = -1,
	SudokuResultOk,
	SudokuResultUnchanged
} SudokuResult;

typedef struct {
	int count;
	int *list;
} NeighbourList;

typedef struct __Sudoku* Sudoku;
struct __Sudoku {
	NeighbourList *neighbour;
	int value[81];
	SudokuNote note[81];
};

typedef struct __SudokuSolution* SudokuSolution;
struct __SudokuSolution {
	int count;
	int *depth;
	int *step;
	Sudoku *list;
};

void SudokuSolutionInit(SudokuSolution*);
void SudokuSolutionDestroy(SudokuSolution);

typedef int(*SudokuGroupGenerator)(int(*)[9]);
typedef SudokuGroupGenerator* SudokuType;



void SudokuInit(Sudoku*, SudokuType);
Sudoku SudokuDuplicate(Sudoku src, int duplicateNeighbourList);
void SudokuCopyNote(Sudoku dst, const Sudoku src);
int  SudokuIsSolved(Sudoku);
void SudokuSetValue(Sudoku, int position, int value);

SudokuResult SudokuUpdateNote(Sudoku);
SudokuResult SudokuSolve(Sudoku, int depth);
SudokuResult SudokuFindSolution(Sudoku sudoku, int solutionLimit, SudokuSolution solution, int depth, int step);

void SudokuPrint(Sudoku);

int SudokuNoteGetValue(SudokuNote);
#define SudokuNoteFromValue(value) (1<<(value))
char SudokuNote2char(SudokuNote);


void NeighbourListPrint(NeighbourList* nl);
void NeighbourListAdd(NeighbourList* dst, int neighbourPosition);
void SudokuAddGroup(Sudoku sudoku, int group[9]);

int SudokuGroupGeneratorVertical(int(*)[9]);
int SudokuGroupGeneratoreHorizontal(int(*)[9]);
int SudokuGroupGeneratorStandardBlock(int(*)[9]);

extern SudokuGroupGenerator SudokuTypeStandard[4];

#endif /* __SUDOKU_H */