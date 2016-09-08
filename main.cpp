#include "agents/agents.h"
#include <ctime>
#include <vector>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <typeinfo>
#include <algorithm>
using namespace std;

int debug;
int regame, game_id;
int num_games = -1;
char *logfile_p, logfile[1010];
int random_seed;
int start_from = 1;

void argparse(int argc, char **argv);

class Table {
private:
	int cards[110];
	int choice[MAX_NUM_PLAYERS], args[MAX_NUM_PLAYERS];
	int random_v1, random_v2, random_a, random_b;
	
	struct Env {
		vector <int> stacks[NUM_STACKS];
		vector <int> score;
		vector <int> handcards[MAX_NUM_PLAYERS];
		int num_players;
		
		Env(int num_players) {
			this->num_players = num_players;
			for (int i = 0; i < NUM_STACKS; ++i) {
				stacks[i].clear();
      }
			score.clear();
			for (int i = 0; i < num_players; ++i) {
				handcards[i].clear();
				score.push_back(0);
			}
		}
		int push(int id, int card, int select = -1) {
			int sid = -1;
			if (select == -1) {
				for (int i = 0; i < NUM_STACKS; ++i)
					if (stacks[i].back() < card)
						if (sid == -1 or stacks[i].back() > stacks[sid].back())
							sid = i;
				if (sid == -1)
					return -1;
			}
			else sid = select;
			if (stacks[sid].back() > card  ||  stacks[sid].size() == STACK_DEPTH) {
				int punish = 0;
				for (int i = 0; i < stacks[sid].size(); ++i)
					punish += NIMMTS[stacks[sid][i]];
				stacks[sid].clear();
				score[id] += punish;
			}
			stacks[sid].push_back(card);
			return 0;
		}
		vector <int> show_handcards(int id) {
			vector <int> ret;
			ret = handcards[id];
			sort(ret.begin(), ret.end());
			return ret;
		}
		void to_playeds(vector <int> &vec) {
			for (int i = 0; i < NUM_STACKS; ++i) {
				for (int j = 0; j < STACK_DEPTH; ++j)
					vec.push_back(j >= stacks[i].size() ? 0 : stacks[i][j]);
			}
		}
	};
	
	vector <int> sum_score(vector <int> score1, vector <int> score2) {
		vector <int> ret;
		ret.resize(score1.size());
		for (unsigned i = 0; i < score1.size(); ++i)
			ret[i] = score1[i] + score2[i];
		return ret;
	}
	int random() {
		random_v1 = ((long long) random_v1 * random_a + random_b) % (int) (1e9 + 7);
		random_v2 = ((long long) random_v2 * random_a + random_b) % (int) (1e9 + 9);
		return random_v1 + random_v2;
	}
	void random_shuffle(int *s, int *t) {
		for (int *i = s; i != t; ++i) {
			int k = this->random() % (i - s + 1);
			swap(*i, *(i - k));
		}
	}
public:
	int game_cnt;

	Table (unsigned seed) {
		random_b = (seed * 1453LL + 326) % 100139009;
		random_a = 1453;
		random_v1 = random_v2 = (seed * 17041 + 173) % (int) (1e9 + 9);
		for (int i = 1; i <= NUM_CARDS; ++i)
			cards[i] = i;
		this->game_cnt = 0;
	}
	void init_new_game() {
		this->game_cnt ++;
		this->random_shuffle(cards + 1, cards + NUM_CARDS + 1);
	}
	vector <int> run(int game_id, int game_rd, vector <Agent*> agents, bool debug = false) {
		this->init_new_game();
		
		int *p = cards + 1;
		int num_players = agents.size();
		vector <int> playeds;
		Env *env = new Env(num_players);
		for (int i = 0; i < 4; ++i) {
			env->stacks[i].push_back(*(p++));
		}
		for (int i = 0; i < num_players; ++i) {
			for (int j = 0; j < NUM_ROUNDS; ++j)
				env->handcards[i].push_back(*(p++));
		}
		for (int i = 0; i < NUM_ROUNDS; ++i) {
			env->to_playeds(playeds);
			for (int j = 0; j < num_players; ++j) {
				choice[j] = agents[j]->policy(j, i, env->show_handcards(j), num_players, NUM_CARDS, playeds, env->score);
				args[j] = j;
			}
			if (debug) {
				printf("============== %d-%d (%d) ROUND %d ================ \n", game_id, game_rd, game_cnt, i);
				printf("\033[1;33mcurrent stacks : \n");
				{
				for (int i = 0; i < NUM_STACKS; ++i) {
					printf("- ");
					for (int j = 0; j < env->stacks[i].size(); ++j)
					printf("%d ", env->stacks[i][j]);
					printf("\n");
				}
				printf("\033[0m");
				vector <int> r;
				for (int i = 0; i < num_players; ++i) {
					printf("\033[1;30magent %d hands : [", i);
					r = env->show_handcards(i); 
					for (int j = 0; j < env->handcards[i].size(); ++j) {
						if (j > 0)
							printf(" ");
						if (r[j] == choice[i])
							printf("\033[1;33m%d\033[0m", r[j]);
						else
							printf("\033[1;30m%d\033[0m", r[j]);
					}
					printf("\033[1;30m]\033[0m\n");
				}
				printf("\033[1;35mscores : [");
				for (int i = 0; i < num_players; ++i) {
					if (i > 0)
						printf(" ");
					printf("%d", env->score[i]);
				}
				printf("]\033[0m \n");
				}
				printf("============ %d-%d (%d) ROUND %d END ============== \n\n", game_id, game_rd, game_cnt, i);
				char c;
				scanf("%c", &c);
			}
			for (int j = 0; j < num_players; ++j)
				for (int k = 1; k < num_players; ++k)
					if (choice[args[k - 1]] > choice[args[k]])
						swap(args[k - 1], args[k]);
					
			for (int j = 0; j < num_players; ++j) {
				playeds.push_back(choice[j]);
				env->handcards[j].erase(find(env->handcards[j].begin(), env->handcards[j].end(), choice[j]));
			}
			for (int j = 0; j < num_players; ++j) {
				int id = args[j];
				int card = choice[id];
				if (env->push(id, card)) {
					int sid = agents[id]->policy_min(id, i + 1, env->show_handcards(id), num_players, NUM_CARDS, playeds, env->score);
					env->push(id, card, sid);
				}
			}
		}
		for (int i = 0; i < num_players; ++i)
			agents[i]->callback(i, NUM_ROUNDS, env->show_handcards(i), num_players, NUM_CARDS, playeds, env->score);
		return env->score;
	}
	vector <int> run66(int game_id, vector <Agent*> agents, bool debug = false) {
		vector <int> score;
		score.resize(agents.size());
		int cnt = 0;
		while (*max_element(score.begin(), score.end()) < 66) {
			score = sum_score(score, run(game_id, cnt, agents, debug));
			++cnt;
			//printf("[%2d %2d]->", score[0], score[1]);
		}
		return score;
	}
	double getpt(vector <int> score) {
		if (score[0] >= 66)
			return score[1] >= 66 ? 1 : 0;
		return 1;
	}
};

int main(int argc, char **argv) {
	char *logfile, logf[1010];

	argparse(argc, argv);
	srand(random_seed);
	
	Table *table = new Table(random_seed);
	vector <Agent*> agents;
	
	Agent *easiest_v2 = new EasiestAgent_v2();
	Agent *easiest = new EasiestAgent();
	Agent *x = new XAgent_v001();
	Agent *kbd = new KbdAgent();
	
	agents.push_back(easiest_v2);
	agents.push_back(x);
	
	for (int i = 0; i < agents.size(); ++i) {
		printf("agents %d : %s \n", i, typeid(*agents[i]).name());
	}
	
	while (table->game_cnt < start_from - 1)
		table->init_new_game();

	int game_id = 0;
	double pt1 = 0;
	while (game_id != num_games) {
		game_id++;
		vector <int> score = table->run66(game_id, agents, debug);
		
		pt1 += table->getpt(score);
		fprintf(stderr, "this game [%2d %2d], total [%.1lf %.1lf]\n", score[0], score[1], pt1, game_id - pt1);
	}
}
void argparse(int argc, char **argv) {
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0 or strcmp(argv[i], "--debug") == 0)
			debug = 1;
		if (strcmp(argv[i], "-n") == 0 or strcmp(argv[i], "--number") == 0) {
			sscanf(argv[++i], "%d", &num_games);
		}
		if (strcmp(argv[i], "-r") == 0 or strcmp(argv[i], "--random") == 0) {
			sscanf(argv[++i], "%d", &random_seed);
		}
		if (strcmp(argv[i], "-s") == 0 or strcmp(argv[i], "--start_from") == 0) {
			sscanf(argv[++i], "%d", &start_from);
		}
		if (strcmp(argv[i], "-h") == 0 or strcmp(argv[i], "--help") == 0) {
			fprintf(stderr, "usage: ./prog_name [-h] [-d] [-s NUM] [-n NUM] [-r NUM]\n\n");
			fprintf(stderr, "optional arguments\n");
			fprintf(stderr, "-h, --help            show this help message and exit\n");
			fprintf(stderr, "-d, --debug           debug mode, will print the situation everytime after all agents decided their cards to show\n");
			fprintf(stderr, "-s, --start_from NUM  generate the game from game_cnt <start_from>\n");
			fprintf(stderr, "-n NUM, --num NUM     set the maximum number of games to run, default is -1, denoting unending games\n");
			fprintf(stderr, "-r NUM, --random NUM  set the random_seed, default is 0.\n");
			exit(0);
		}
	}
}