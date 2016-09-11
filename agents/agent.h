#ifndef __AGENT_HEAD_FILE_
#define __AGENT_HEAD_FILE_  

#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <typeinfo>
#define MAX_NUM_PLAYERS 4
#define NUM_STACKS 4
#define NUM_ROUNDS 10
#define NUM_CARDS 104
#define STACK_DEPTH 5
using namespace std;

const int NIMMTS[105] = {0, 1, 1, 1, 1, 2, 1, 1, 1, 1, 3, 5, 1, 1, 1, 2, 1, 1, 1, 1, 3, 1, 5, 1, 1, 2, 1, 1, 1, 1, 3, 1, 1, 5, 1, 2, 1, 1, 1, 1, 3, 1, 1, 1, 5, 2, 1, 1, 1, 1, 3, 1, 1, 1, 1, 7, 1, 1, 1, 1, 3, 1, 1, 1, 1, 2, 5, 1, 1, 1, 3, 1, 1, 1, 1, 2, 1, 5, 1, 1, 3, 1, 1, 1, 1, 2, 1, 1, 5, 1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 5, 3, 1, 1, 1, 1};
class Agent {
public :
	virtual int policy(int player_id, int current_round, std::vector <int> hands, int num_players, int num_cards, std::vector <int> playeds, std::vector <int> scores) = 0;
	virtual int policy_min(int player_id, int current_round, std::vector <int> hands, int num_players, int num_cards, std::vector <int> playeds, std::vector <int> scores) = 0;
	virtual void callback(int player_id, int current_round, std::vector <int> hands, int num_players, int num_cards, std::vector <int> playeds, std::vector <int> scores) {
		return;
	}
};
map <string, Agent*> agents_set;

void add_to_agents_set(const string &name, Agent *agent, bool overlap = true) {
	if (agents_set.find(name) != agents_set.end()) {
		printf("duplicated agent %s. \n");
		Agent *x = agents_set[name], *y = agent;
		if (overlap == true) {
			printf("agent [%s] : overwrite %s by %s \n", name.c_str(), typeid(*y).name(), typeid(*x).name());
			agents_set[name] = y; 
		}
		else
			printf("agent [%s] : ignore %s. \n", name.c_str(), typeid(*y).name());
	}
	else agents_set[name] = agent;
}
class AgentsRegister {
public : 
	AgentsRegister(const string &name, Agent *x, bool overlap = true) {
		add_to_agents_set(name, x, overlap);
	}
};

#endif