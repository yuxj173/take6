/*
	agent : KbdAgent (Human Player)
	use it to play take6!
*/

#include "agent.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#define NUM_PLAYERS 2
using namespace std;

class KbdAgent : public Agent {
private:
	int stor_arr[2010], *stor;
	int handcards[10], num_handcards;
	int bar[20];
	bool shows[110];
	int _;
	int maxdepth;
	int last_choice[10];
	struct Env {
		int *stacks[NUM_STACKS], nimmts[NUM_STACKS], scores[NUM_PLAYERS];
	};
	Env *env, stor_env[10];
public:
	KbdAgent() {
		env = new Env();
		stor = stor_arr;
		for (int i = 0; i < NUM_PLAYERS; ++i) {
			env->scores[i] = 0;
		}
		for (int i = 0; i < NUM_STACKS; ++i) {
			env->stacks[i] = stor;
			stor += STACK_DEPTH + 1;
		}
	}
	void Init(int pid, int rd, vector <int> __handcards, int num_players, int num_cards, vector <int> __cards, bool small_mode, vector <int> scores) {
		int card, cid = 0;
		num_handcards = NUM_ROUNDS - rd;
		for (int i = 0; i < num_handcards; ++i) {
			handcards[i] = __handcards[i];
		}
		sort(handcards, handcards + num_handcards);
			
		rd -= small_mode;
		for (int i = 0; i <= rd; ++i) {
			for (int j = 0; j < NUM_STACKS; ++j) {
				env->stacks[j][0] = 0;
				env->nimmts[j] = 0;
				for (int k = 0; k < STACK_DEPTH; ++k) {
					card = __cards[cid++];
					env->stacks[j][k + 1] = card;
					env->stacks[j][0] += (card != 0);
					env->nimmts[j] += NIMMTS[card];
				}
			}
			if ((i != rd) || (small_mode == 1)) {
				for (int j = 0; j < num_players; ++j) {					
					card = __cards[cid++];
					last_choice[j] = card;
				}
			}
		}
		for (int i = 0; i < num_players; ++i)
			env->scores[i] = scores[i];
	}
	int policy(int pid, int rd, vector <int> handcards, int num_players, int num_cards, vector <int> cards, vector <int> scores) {
		Init(pid, rd, handcards, num_players, num_cards,cards, 0, scores);
		printf("current stacks : \n");
		for (int i = 0; i < NUM_STACKS; ++i) {
			printf("- ");
			for (int j = 1; j <= env->stacks[i][0]; ++j)
				printf("%d ", env->stacks[i][j]);
			printf("\n");
		}
		printf("your handcards : ");
		for (int i = 0; i < num_handcards; ++i) {
			printf("%d ", handcards[i]);
		}
		printf("\n");
		printf("scores : ");
		for (int i = 0; i < num_players; ++i)
			printf("%d ", scores[i]);
		printf("\n");
		printf("your choice : ");
		while (true) {
			int x;
			scanf("%d", &x);
			for (int i = 0; i < num_handcards; ++i)
				if (x == handcards[i])
					return x;
		}
	}
	int policy_min(int pid, int rd, vector <int> handcards, int num_players, int num_cards, vector <int> cards, vector <int> scores) {
		Init(pid, rd, handcards, num_players, num_cards, cards, 1, scores);
		printf("current stacks : \n");
		for (int i = 0; i < NUM_STACKS; ++i) {
			printf("- ");
			for (int j = 1; j <= env->stacks[i][0]; ++j)
				printf("%d ", env->stacks[i][j]);
			printf("\n");
		}
		printf("your handcards : ");
		for (int i = 0; i < num_handcards; ++i) {
			printf("%d ", handcards[i]);
		}
		printf("\n");
		printf("scores : ");
		for (int i = 0; i < num_players; ++i)
			printf("%d ", scores[i]);
		printf("\n");
		printf("showed cards : ");
		for (int i = 0; i < num_players; ++i) {
			printf("%d ", last_choice[i]);
		}
		printf("\n");
		printf("select a stack (your id is %d): ", pid);
		while (true) {
			int x;
			scanf("%d", &x);
			if (x >= 0  &&  x < NUM_STACKS)
				return x;
		}		
	}
	void callback(int pid, int rd, vector <int> handcards, int num_players, int num_cards, vector <int> cards, vector <int> scores) {
		Init(pid, rd, handcards, num_players, num_cards, cards, 1, scores);
		printf("final stacks : \n");
		for (int i = 0; i < NUM_STACKS; ++i) {
			printf("- ");
			for (int j = 1; j <= env->stacks[i][0]; ++j)
				printf("%d ", env->stacks[i][j]);
			printf("\n");
		}
		printf("final scores : ");
		for (int i = 0; i < num_players; ++i)
			printf("%d ", scores[i]);
		printf("\n");
		printf("===========================================\n");
		char _;
		scanf("%c", &_);
	}
};
/*
int main(int argc, char **argv) {
	NaiveAgent1v1 *agent = new NaiveAgent1v1();
}
*/



