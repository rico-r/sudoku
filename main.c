
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "sudoku.h"

FILE *openFile(const char* path, int read){
	char* msg;
	if(!access(path, F_OK)){
		struct stat st;
		stat(path, &st);
		if(S_ISDIR(st.st_mode)){
			msg="Is directory";
			goto err;
		}
		if(access(path, read ? R_OK : W_OK)){
			msg = "Permission denied";
			goto err;
		}
	}else{
		if(read){
			msg = "No such file or directory";
			goto err;
		}
	}
	return fopen(path, read?"rb":"wb");
	
	err:
	fprintf(stderr, "%s: %s\n", path, msg);
	exit(1);
}

int SudokuLoadFile(Sudoku dst, FILE* f){
	int i = 0;
	while(!feof(f) && i<81){
		char v = getc(f);
		if(v>='1' && v<='9'){
			dst->note[i] = SudokuNoteFromValue(v-'1') | SudokuNoteFixed;
			dst->value[i] = v-'1';
			i++;
		}else if(v=='.'){
			i++;
		}
	}
	return i==81;
}

int SudokuGetUnique(int *dst, SudokuSolution solution, int i){
	Sudoku sudoku=solution->list[i];
	int result=0;
	for(int j=0; j<81; j++){
		int isUnique=solution->count!=1;
		for(int k=0; k<solution->count; k++){
			if(i==k){
				// skip self
				continue;
			}
			isUnique &= sudoku->note[j] != solution->list[k]->note[j];
		}
		result+=dst[j]=isUnique;
	}
	return solution->count==1?1:result;
}

void SudokuPrintSolution(Sudoku sudoku, int *unique){
	for(int y=0; y<9; y++){
		for(int x=0; x<9; x++){
			int position=y*9+x;
			int v=sudoku->note[position];
			if(v&SudokuNoteFixed){
				// highlight fixed note
				printf("\e[3m%c\e[23m", SudokuNote2char(v));
			}else{
				if(unique[position]){
					printf("\e[48;5;241m%c\e[m", SudokuNote2char(v));
				}else{
					putchar(SudokuNote2char(v));
				}
			}
			putchar(' ');
			// vertical space
			if(x%3==2) putchar(' ');
		}
		putchar('\n');
		// horizontal space
		if(y%3==2 && y<8) putchar('\n');
	}
}

void findSolution(Sudoku sudoku){
	SudokuSolution solution;
	SudokuSolutionInit(&solution);
	SudokuFindSolution(sudoku, 0, solution, 0, 0);
	if(solution->count>1){
		printf("%d solution found\n", solution->count);
	}
	int unique[81];
	int uniqueCount=0;
	for(int i=0; i<solution->count; i++){
		if(!SudokuGetUnique(unique, solution, i)){
			continue;
		}
		if(i!=0) puts("=====================");
		SudokuPrintSolution(solution->list[i], unique);
		uniqueCount++;
	}
	if(solution->count>1){
		puts("======================");
		printf("%d unique\n", uniqueCount);
	}
	SudokuSolutionDestroy(solution);
}

int SudokuCountSolution(Sudoku sudoku);
int SudokuCountSolution(Sudoku sudoku){
	int r;
	while((r=SudokuUpdateNote(sudoku))==SudokuResultOk);
	if(r==SudokuResultError){
		return 0;
	}
	if(SudokuIsSolved(sudoku)){
		return 1;
	}

	// find next fillable note
	register int position=-1;
	for(register int i=0; i<81; i++){
		if(sudoku->note[i]&SudokuNoteOk){
			continue;
		}
		position=i;
		break;
	}
	int note = sudoku->note[position];
	int value = -1;

	struct __Sudoku copy[1];
	copy->neighbour = sudoku->neighbour;
	int solved = 0;
	do{
		value++;
		if(!(note&1))continue;
		SudokuCopyNote(copy, sudoku);
		SudokuSetValue(copy, position, value);
		solved += SudokuCountSolution(copy);
	}while(note>>=1);
	return solved;
}

#define USAGE \
"sudoku [-c] [-p] [--] [file]\n"\
"  Solve sudoku and print all solution found\n"\
"-h    Show this help\n"\
"-p    Print inputed sudoku\n"\
"-c    Only print the number of solution found\n"

int main(int argc, const char**argv){
	int countOnly = 0;
	int printInput = 0;
	int stopParseOption = 0;
	FILE *input = stdin;

	for(int i=1; i<argc; i++){
		const char* v = argv[i];
		if(stopParseOption) goto set_input;
		else if(!strcmp(v, "-c")) countOnly = 1;
		else if(!strcmp(v, "-p")) printInput = 1;
		else if(!strcmp(v, "-h")) goto usage;
		else if(!strcmp(v, "--")) stopParseOption = 1;
		else{
			set_input:
			input = openFile(argv[i], 1);
		}
	}
	
	Sudoku sudoku;
	SudokuInit(&sudoku, SudokuTypeStandard);
	SudokuLoadFile(sudoku, input);
	
	if(printInput){
		SudokuPrint(sudoku);
		puts("=====================");
	}

	if(countOnly){
		printf("%d Solution found\n", SudokuCountSolution(sudoku));
	}else{
		findSolution(sudoku);
	}
	return 0;

	usage:
	fputs(USAGE, stderr);
	exit(1);
}