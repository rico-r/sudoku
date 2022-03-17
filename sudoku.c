
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sudoku.h"

#define DefineGroupGenerator(name, count, value) \
	int SudokuGroupGenerator##name(int (*__result)[9]){\
		for(int y=0; y<count; y++){\
			for(int x=0; x<9; x++){\
				__result[y][x] = value;\
			}\
		}\
		return count;\
	}

DefineGroupGenerator(Vertical, 9, y*9+x)
DefineGroupGenerator(Horizontal, 9, x*9+y)
DefineGroupGenerator(StandardBlock, 9, (y/3*3+x/3)*9 + y%3*3+x%3)

SudokuGroupGenerator SudokuTypeStandard[4] = {
	SudokuGroupGeneratorVertical,
	SudokuGroupGeneratorHorizontal,
	SudokuGroupGeneratorStandardBlock,
	NULL};

void SudokuInit(Sudoku* dst, SudokuType type){
	Sudoku sudoku = *dst = (Sudoku) malloc(sizeof(struct __Sudoku));
	sudoku->neighbour = malloc(81*sizeof(NeighbourList));
	for(int i=0; i<81; i++){
		sudoku->note[i] = SudokuNoteAll;
		sudoku->value[i] = 0;
		sudoku->neighbour[i].count = 0;
		sudoku->neighbour[i].list = malloc(0);
	}
	int i=0;
	int groups[9][9];
	while(type[i]){
		int count = type[i](groups);
		for(int j=0; j<count; j++){
			SudokuAddGroup(sudoku, groups[j]);
		}
		i++;
	}
}

Sudoku SudokuDuplicate(Sudoku src, int duplicateNeighbourList){
	Sudoku sudoku = (Sudoku) malloc(sizeof(struct __Sudoku));
	memcpy(sudoku->note, src->note, 81*sizeof(SudokuNote));
	memcpy(sudoku->value, src->value, 81*sizeof(int));
	if(duplicateNeighbourList){
		// TODO: implement this block
		int TODO_implement_this_block = 0;
		assert(TODO_implement_this_block);
	}else{
		sudoku->neighbour = src->neighbour;
	}
	return sudoku;
}

void SudokuCopyNote(Sudoku dst, const Sudoku src){
	memcpy(dst->note, src->note, 81*sizeof(SudokuNote));
	memcpy(dst->value, src->value, 81*sizeof(int));
}

int SudokuIsSolved(Sudoku sudoku){
	for(int i=0; i<81; i++){
		if((sudoku->note[i]&SudokuNoteOk)==0){
			return 0;
		}
	}
	return 1;
}

void SudokuSetValue(Sudoku sudoku, int position, int value){
	int reverseValue=~SudokuNoteFromValue(value);
	int *neighbour = sudoku->neighbour[position].list;
	int count = sudoku->neighbour[position].count;
	for(int i=0; i<count; i++){
		sudoku->note[neighbour[i]]&=reverseValue;
	}
	sudoku->note[position]=~reverseValue | SudokuNoteOk | (sudoku->note[position]&SudokuNoteFixed);
	sudoku->value[position]=value;
}

SudokuResult SudokuUpdateNote(Sudoku sudoku){
	SudokuResult result = SudokuResultUnchanged;
	int i, value;
	for(i=0; i<81; i++){
		SudokuNote note = sudoku->note[i];
		if(0==(note&SudokuNoteAll)){
			return SudokuResultError;
		}
		if(note&SudokuNoteOk){
			continue;
		}

		value = SudokuNoteGetValue(note);
		if(value==-1){
			continue;
		}
		SudokuSetValue(sudoku, i, value);
		result=SudokuResultOk;
	}
	return result;
}

SudokuResult SudokuSolve(Sudoku sudoku, int depth){
	SudokuResult result;
	int step=-1;
	do{
		result=SudokuUpdateNote(sudoku);
		step++;
	}while(result==SudokuResultOk);
	if(result==SudokuResultError){
		return SudokuResultError;
	}
	
	if(SudokuIsSolved(sudoku)){
		return SudokuResultOk;
	}

	// find next fillable note
	int position = -1, note = 0;
	int value = -1;
	struct __Sudoku copy[1];
	for(int i=0;i<81;i++){
		if(0==(sudoku->note[i]&SudokuNoteOk)){
			position=i;
			note = sudoku->note[i];
			break;
		}
	}

	copy->neighbour = sudoku->neighbour;
	do{
		value++;
		if(!(note&1))continue;
		SudokuCopyNote(copy, sudoku);
		SudokuSetValue(copy, position, value);
		if(SudokuSolve(copy, depth+1)==SudokuResultOk){
			SudokuCopyNote(sudoku, copy);
			return SudokuResultOk;
		}
	}while((note>>=1)&&value<8);
	return SudokuResultError;
}

SudokuResult SudokuFindSolution(Sudoku sudoku, int solutionLimit, SudokuSolution solution, int depth, int step){
	int r;
	while((r=SudokuUpdateNote(sudoku))==SudokuResultOk){
		step++;
	}
	if(r==SudokuResultError){
		return SudokuResultError;
	}
	if(SudokuIsSolved(sudoku)){
		int count=++solution->count;
		solution->depth=realloc(solution->depth, count*sizeof(int));
		solution->step=realloc(solution->step, count*sizeof(int));
		solution->list=realloc(solution->list, count*sizeof(Sudoku));
		int i=count-1;
		solution->depth[i]=depth;
		solution->step[i]=step;
		solution->list[i]=SudokuDuplicate(sudoku, 0);
		return SudokuResultOk;
	}

	// find next fillable note
	int position=-1;
	for(int i=0; i<81; i++){
		if(sudoku->note[i]&SudokuNoteOk){
			continue;
		}
		position=i;
		break;
	}
/*	int fillableList[81];
	int n=0;
	int next=rand();
	for(int i=0; i<81; i++){
		if(sudoku->note[i]&SudokuNoteOk){
			continue;
		}
		if(n==next){
			position=i;
			break;
		}
		fillableList[n++]=i;
	}
	if(position==-1){
		position=fillableList[next%n];
	}*/
	int note = sudoku->note[position];
	int value = -1;

	struct __Sudoku copy[1];
	copy->neighbour = sudoku->neighbour;
	do{
		value++;
		if(!(note&1))continue;
		SudokuCopyNote(copy, sudoku);
		SudokuSetValue(copy, position, value);
		SudokuFindSolution(copy, solutionLimit, solution, depth+1, step);
		if(solutionLimit){
			if(solution->count>=solutionLimit){
				return SudokuResultOk;
			}
		}
	}while(note>>=1);
	return solution->count? SudokuResultOk : SudokuResultError;
}

void SudokuPrint(Sudoku sudoku){
	for(int y=0; y<9; y++){
		for(int x=0; x<9; x++){
			int v=sudoku->note[y*9+x];
			if(v&SudokuNoteFixed){
				// highlight fixed note
				printf("\e[3m%c\e[23m", SudokuNote2char(v));
			}else{
				putchar(SudokuNote2char(v));
			}
			putchar(' ');
			// vertical space
			if(x%3==2)putchar(' ');
		}
		putchar('\n');
		// horizontal space
		if(y%3==2 && y<8)putchar('\n');
	}
}

int SudokuNoteGetValue(SudokuNote v){
	switch(v&SudokuNoteAll){
		case SudokuNote1: return 0;
		case SudokuNote2: return 1;
		case SudokuNote3: return 2;
		case SudokuNote4: return 3;
		case SudokuNote5: return 4;
		case SudokuNote6: return 5;
		case SudokuNote7: return 6;
		case SudokuNote8: return 7;
		case SudokuNote9: return 8;
		default: return -1;
	}
}

char SudokuNote2char(SudokuNote v){
	switch(v&SudokuNoteAll){
		case 0:  return '?';
		case SudokuNote1: return '1';
		case SudokuNote2: return '2';
		case SudokuNote3: return '3';
		case SudokuNote4: return '4';
		case SudokuNote5: return '5';
		case SudokuNote6: return '6';
		case SudokuNote7: return '7';
		case SudokuNote8: return '8';
		case SudokuNote9: return '9';
		default: return '.';
	}
}

void SudokuSolutionInit(SudokuSolution* dst){
	SudokuSolution s = *dst = malloc(sizeof(struct __SudokuSolution));
	s->count = 0;
	s->depth = malloc(0);
	s->step = malloc(0);
	s->list = malloc(0);
}

void SudokuSolutionDestroy(SudokuSolution s){
	int count=s->count;
	for(int i=0; i<count; i++){
		free(s->list[i]);
	}
	free(s->depth);
	free(s->step);
	free(s->list);
	free(s);
}

void NeighbourListPrint(NeighbourList* nl){
	for(int i=0; i<81; i++){
		int* list=nl[i].list;
		int count=nl[i].count;
		printf("[%2d]=", i);
		for(int j=0; j<count; j++){
			if(j){
				printf(", ");
			}
			printf("%2d", list[j]);
		}
		putchar('\n');
	}
}

void NeighbourListAdd(NeighbourList* dst, int neighbourPosition){
	int count = dst->count;
	for(int i=0;i<count;i++){
		if(dst->list[i]==neighbourPosition){
			// allready exist, skip
			return;
		}
	}
	dst->list = realloc(dst->list, (++dst->count)*sizeof(*dst->list));
	dst->list[count] = neighbourPosition;
}

void SudokuAddGroup(Sudoku sudoku, int group[9]){
	for(int i=0; i<9; i++){
		NeighbourList* neighbourList = &sudoku->neighbour[group[i]];
		for(int j=0; j<9; j++){
			if(i==j) continue;
			NeighbourListAdd(neighbourList, group[j]);
		}
	}
}