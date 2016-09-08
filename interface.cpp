#include "agents/agents.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
using namespace std;
 
int main(int argc, char **argv) {
	vector <int> handcards;
	vector <int> cards;
	vector <int> scores(2, 0);
	
	int pid, rd, num_players, num_cards;
	int card;
	int small_mode = (argc == 2) && (argv[1][1] == 's');
	int num_handcards;
	
	cin >> pid >> rd;
	num_handcards = NUM_ROUNDS - rd;
	for (int i = 0; i < num_handcards; ++i) {
		cin >> card;
		handcards.push_back(card);
	}
	if (num_handcards)
		sort(handcards.begin(), handcards.end());
	
	cin >> num_players >> num_cards;
	rd -= small_mode;
	for (int i = 0; i <= rd; ++i) {
		for (int j = 0; j < NUM_STACKS; ++j)
			for (int k = 0; k < STACK_DEPTH; ++k) {
				cin >> card;
				cards.push_back(card);
			}
		if ((i != rd) || (small_mode == 1)) {
			for (int j = 0; j < num_players; ++j) {
				cin >> card;
				cards.push_back(card);
			}
		}
	}
	
	Agent *agent = new XAgent_v001(2);
	
	int ret;
	if (small_mode)
		ret = agent->policy_min(pid, rd + small_mode, handcards, num_players, num_cards, cards, scores);
	else
		ret = agent->policy(pid, rd, handcards, num_players, num_cards, cards, scores);
	cout << ret << endl;
}



