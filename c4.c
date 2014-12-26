//c4.c - Connect-4 (C style)
//Author: Shane Barratt, 12/25/2014

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

typedef int64_t int64;

#define TRUE 1
#define FALSE 0

#define P2 1
#define P1 0

char col_to_new[64];

#define mask_t1 (int64)0b011111011111011111011111011111011111011111
#define mask_t2 (int64)0b001111001111001111001111001111001111001111
#define mask_t3 (int64)0b000111000111000111000111000111000111000111

#define mask_r (int64)0b111000111100111110111111011111001111000111
#define mask_l (int64)0b000111001111011111111111111110111100111000

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
/*

int64 one = 1;
x1 = one << 5 | one<< (5+6*1) | one << (5+6*2) | one << (5+6*3) | one << (5+6*4) | one << (5+6*5) | one << (5+6*6);
x2 = x1 | x1 >> 1;
x3 = x2 | x2 >> 1;


*/

void print_board(int64 p1, int64 p2) {
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j <= 6; j++) {
			int shamt = (36+i-6*j);
			if (((p1 >> shamt) & 1) == 1){
				printf("X");
			}
			else if (((p2 >> shamt) & 1) == 1) {
				printf("O");
			}
			else {
				printf("_");
			}
		}
		printf("\n");
	}
}

char connect_four(int64 b) {
	if (b & (b >> 6) & (b >> 12) & (b >> 18)) {
		return TRUE;
	}
	if(b & ((b >> 1) & mask_t1) & ((b >> 2) & mask_t2) & ((b >> 3) & mask_t3)) {
		return TRUE;
	}
	int64 b_r = b & mask_r;
	if(b_r & (b_r >> 7) & (b_r >> 14) & (b_r >> 21)) {
		return TRUE;
	}
	int64 b_l = b & mask_l;
	if(b_l & (b_l << 5) & (b_l << 10) & (b_l << 15)) {
		return TRUE;
	}
	return FALSE;
}

int64 add_piece(int64 b, int64 both, int col) { //col in {0,1,...,6}
	int64 resulting_col = col_to_new[(both >> ((6-col)*6)) & 0x3F];
	char x = 6*(6-col)+5-resulting_col;
	return b | ((int64)(1) << x);
}

char valid_move(int64 b, int col) {
	return ((b >> ((6-col)*6)) & 0x3F) != 0b111111;
}

void playgame() {
	int64 p1 = 0;
	int64 p2 = 0;

	int col;
	int turn = P1;
	int gameover = FALSE;

	while (!gameover) {
		col = -1;
		print_board(p1,p2);
		while (col < 0 || col > 6 || !valid_move(p1|p2,col)) {
			printf("P1, please input a column to place your piece:");
			scanf("%d",&col);
			if (col == EOF) {
				printf("\n");
				exit(0);
			}
		}
		if (turn == P1) {
			p1 = add_piece(p1,p1|p2,col);
			if (connect_four(p1)) {
				printf("\nPlayer 1 Wins\n");
				print_board(p1,p2);
				gameover = TRUE;
			}
		}
		else if (turn == P2) {
			p2 = add_piece(p2,p1|p2,col);
			if (connect_four(p2)) {
				printf("\nPlayer 2 Wins\n");
				print_board(p1,p2);
				gameover = TRUE;
			}
		}
		turn = 1 - turn;
	}
}

/*
Best benchmark:
gcc c4.c ; time ./a.out
real	0m13.772s
user	0m13.668s
*/
void benchmark() {
	int64 p1 = 0;

	for(int i = 0; i < 1000000000; i++) {
		connect_four(p1);
		add_piece(p1,p1,1);
	}
}

int64 eval(int64 p1, int64 p2) {
	return -p2&(int64)0b000000100000110000111000110000100000000000;
}

int minimax(int64 p1,int64 p2,int depth, char maximizingPlayer) {
	if (connect_four(p1)) {
		return INT_MAX-1;
	}
	else if(connect_four(p2)) {
		return INT_MIN+1;
	}
	if (depth == 0) {
		return eval(p1,p2);
	}
	if (maximizingPlayer) {
		int bestValue = INT_MIN;
		for (int col = 0; col < 7; col++) {
			if (valid_move(p1|p2,col)) {
				int64 child = add_piece(p1,p1|p2,col);
				int val = minimax(child,p2,depth-1,FALSE);
				bestValue = MAX(bestValue,val);
			}
		}
		return bestValue;
	} else {
		int bestValue = INT_MAX;
		for (int col = 0; col < 7; col++) {
			if (valid_move(p1|p2,col)) {
				int64 child = add_piece(p2,p1|p2,col);
				int val = minimax(p1,child,depth-1,TRUE);
				bestValue = MIN(bestValue,val);
			}
		}
		return bestValue;
	}
}

int alphabeta(int64 p1,int64 p2,int alpha, int beta,int depth, char maximizingPlayer) {
	if (connect_four(p1)) {
		return INT_MAX-1;
	}
	else if(connect_four(p2)) {
		return INT_MIN+1;
	}
	if (depth == 0) {
		return eval(p1,p2);
	}
	if (maximizingPlayer) {
		for (int col = 0; col < 7; col++) {
			if (valid_move(p1|p2,col)) {
				int64 child = add_piece(p1,p1|p2,col);
				alpha = MAX(alpha,alphabeta(child,p2,alpha,beta,depth-1,FALSE));
				if (beta <= alpha) {
					break;
				}
			}
		}
		return alpha;
	} else {
		for (int col = 0; col < 7; col++) {
			if (valid_move(p1|p2,col)) {
				int64 child = add_piece(p2,p1|p2,col);
				beta = MIN(beta,alphabeta(p1,child,alpha,beta,depth-1,TRUE));
				if (beta <= alpha) {
					break;
				}
			}
		}
		return beta;
	}
}

int get_best_move_p2(int64 p1, int64 p2) {
	int best_move = 0;
	int best_score = INT_MAX;
	for(int i = 0; i < 7; i++) {
		if (valid_move(p1|p2,i)) {
			int64 new_b = add_piece(p2,p1|p2,i);
			// int score = minimax(p1,new_b,8,TRUE);
			int score = alphabeta(p1,new_b,INT_MIN,INT_MAX,8,TRUE);
			if (score < best_score) {
				best_move = i;
				best_score = score;
			}
		}
	}
	return best_move;
}

int main() {
	col_to_new[0b000000] = 0;
	col_to_new[0b100000] = 1;
	col_to_new[0b110000] = 2;
	col_to_new[0b111000] = 3;
	col_to_new[0b111100] = 4;
	col_to_new[0b111110] = 5;
	col_to_new[0b111111] = 5;
	// printf("%d\n",minimax(0,0,10,TRUE));

	int64 p1 = 0;
	int64 p2 = 0;

	int col;
	int turn = P1;
	int gameover = FALSE;

	while (!gameover) {
		col = -1;
		print_board(p1,p2);
		while (turn == P1 && (col < 0 || col > 6 || !valid_move(p1|p2,col))) {
			for(int i = 0; i < 7; i++) {
				printf("%d",i);
			}
			printf("\n\n");

			printf("P1, please input a column to place your piece:");
			scanf("%d",&col);
			if (col == EOF) {
				printf("\n");
				exit(0);
			}
		}
		if (turn == P1) {
			p1 = add_piece(p1,p1|p2,col);
			if (connect_four(p1)) {
				printf("\nPlayer 1 Wins\n");
				print_board(p1,p2);
				gameover = TRUE;
			}
		}
		else if (turn == P2) {
			p2 = add_piece(p2,p1|p2,get_best_move_p2(p1,p2));
			printf("\n");
			if (connect_four(p2)) {
				printf("\nPlayer 2 Wins\n");
				print_board(p1,p2);
				gameover = TRUE;
			}
		}
		turn = 1 - turn;
	}
}