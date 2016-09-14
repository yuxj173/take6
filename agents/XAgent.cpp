/*
	agent : XAgent
	version : 0.01
	author : YuXJ173
	level : easy
	tags : search + evaluation referenced to EasiestAgent
	
	current result :
		83.9~85.3% beats EasiestAgent
		40%		   beats wph
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

class XAgent_v001 : public Agent {
private:
	int stor_arr[2010], *stor;
	int handcards[10], num_handcards;
	int pre_sum[110], tot_cards;
	int stor_min_id_nimmts;
	int bar[20];
	bool shows[110];
	int _;
	double log_c[110][110], log_fac[110];
	int maxdepth;
	int stor_segs_card[10][110];
	double stor_segs_prob[10][110];
	double prob_single[20][110];
	double prob_all[20][110];
	double prob_double[20][110];
	int ppp[20];
	struct Env {
		int *stacks[NUM_STACKS], nimmts[NUM_STACKS], scores[NUM_PLAYERS];

		inline int min_id() {
			return min_element(nimmts, nimmts + NUM_STACKS) - nimmts;
		}
		inline int to(const int &card) {
			int *s;
			int min_top = -1, sid = -1;
			for (int i = 0; i < NUM_STACKS; ++i) {
				s = stacks[i];
				if (s[s[0]] < card  &&  s[s[0]] > min_top)
					min_top = s[s[0]], sid = i;
			}
			return sid;
		}
		inline void sort() {
			for (int i = 0; i < NUM_STACKS; ++i)
				for (int j = 1; j < NUM_STACKS - i; ++j)
					if (stacks[j - 1][stacks[j - 1][0]] > stacks[j][stacks[j][0]])
						swap(stacks[j], stacks[j - 1]), swap(nimmts[j], nimmts[j - 1]);
		}
		inline int push(const int &card, const int &pid, int *stor, int select = -1) {
			int sid = to(card), punish = 0;
			if (sid == -1)
				sid = select == -1 ? min_id() : select;
			int *s = stacks[sid];
			if (s[s[0]] > card  ||  s[0] == STACK_DEPTH) {
				stor[0] = 1, stor[1] = card;
				punish = nimmts[sid];
				nimmts[sid] = NIMMTS[card];
			}
			else {
				for (int i = 0; i <= s[0]; ++i)
					stor[i] = s[i];
				stor[0]++;
				stor[stor[0]] = card;
				nimmts[sid] += NIMMTS[card];
			}
			stacks[sid] = stor;
			scores[pid] += punish;
		}
		inline void push2(const int &card1, const int &card2, int *stor) {
			int s1 = to(card1), s2 = to(card2);
			if (s1 == -1  &&  s2 != -1) {
				int x = min_id();
				if (x == s2  &&  (s2 == 0  ||  stacks[s2 - 1][0] != 5)  &&  stacks[s2][0] == 5) {
					int idx = x;
					for (int i = 0; i < NUM_STACKS; ++i)
						if (i != s2  &&  (idx == s2  || nimmts[i] < nimmts[idx]))
							idx = i;
					if (nimmts[idx] - nimmts[x] < nimmts[x])
						x = idx;
				}
				else if (x != s2  &&  (s2 != 0  &&  	stacks[s2 - 1][0] == 5)) {
					if (stacks[s2][0] != 5) {
						if (nimmts[x] > nimmts[s2] - nimmts[s2 - 1]) x = s2;
					}
					else {
						if (nimmts[x] - nimmts[s2] > nimmts[s2] - nimmts[s2 - 1]) x = s2;
					}
				}
				push(card1, 0, stor, x);
				push(card2, 1, stor + (STACK_DEPTH + 1));
			}
			else if (s2 == -1  &&  s1 != -1) {
				int x = min_id();
				if (x == s1  &&  (s1 == 0  ||  stacks[s1 - 1][0] != 5)  &&  stacks[s1][0] == 5) {
					int idx = x;
					for (int i = 0; i < NUM_STACKS; ++i)
						if (i != s1  &&  (idx == s1  || nimmts[i] < nimmts[idx]))
							idx = i;
					if (nimmts[idx] - nimmts[x] < nimmts[x])
						x = idx;
				}
				else if (x != s1  &&  (s1 != 0  &&  stacks[s1 - 1][0] == 5)) {
					if (stacks[s1][0] != 5) {
						if (nimmts[x] > nimmts[s1] - nimmts[s1 - 1]) x = s1;
					}
					else {
						if (nimmts[x] - nimmts[s1] > nimmts[s1] - nimmts[s1 - 1]) x = s1;
					}
				}
				push(card2, 1, stor, x);
				push(card1, 0, stor + (STACK_DEPTH + 1));
			}
			else if (card1 < card2)
				push(card1, 0, stor), push(card2, 1, stor + (STACK_DEPTH + 1));
			else push(card2, 1, stor), push(card1, 0, stor + (STACK_DEPTH + 1));
		}
	};
	Env *env, stor_env[10];

	inline double evaluate(Env *env, const int &card, int next_card) {
		int sid = env->to(card);
		double e = 0;
		if (sid == -1)
			e = (1.0 - (double) pre_sum[card - 1] / tot_cards) * env->nimmts[env->min_id()];
		else {
			int *s = env->stacks[sid];
			if (s[0] == STACK_DEPTH)
				e = (1.0 - (double) (pre_sum[card - 1] - pre_sum[s[s[0]]]) / tot_cards) * env->nimmts[sid];
			else {
				e = (-0.1 + pre_sum[card - 1] - pre_sum[s[s[0]]]) / tot_cards / (STACK_DEPTH - s[0]) * env->nimmts[sid];
				for (int i = 0; i < NUM_STACKS; ++i)
					if (env->stacks[i][env->stacks[i][0]] > card)
						next_card = min(next_card, env->stacks[i][env->stacks[i][0]]);
				if (s[0] == 4)
					e -= (double) (pre_sum[next_card - 1] - pre_sum[card]) / tot_cards * (env->nimmts[sid] + NIMMTS[card]);
				else
					e += (double) (pre_sum[next_card - 1] - pre_sum[card]) / tot_cards * (env->nimmts[sid] + NIMMTS[card]);
			}
		}
		return e;
	}
	int gen_segs_to(int *card, double *prob, Env *env) {
		stor_min_id_nimmts = env->min_id();
		int cnt = 0, num = 0;
		double maxi = 0, tot = 0;
		bar[0] = 0;
		for (int i = 0; i < num_handcards; ++i)
			bar[++num] = handcards[i];
		for (int i = 0; i < NUM_STACKS; ++i)
			bar[++num] = env->stacks[i][env->stacks[i][0]];
		bar[++num] = NUM_CARDS + 1;
		sort(bar, bar + num + 1);
		for (int i = 0; i < num; ++i) {
			int mdc = -1, cnt2 = 0, cnt3 = 0;
			for (int j = bar[i] + 1; j < bar[i + 1]; ++j) {
				if (shows[j] == false)
					++cnt2;
			}
			if (cnt2 == 0)
				continue;
			cnt2 = (cnt2 + 1) / 2;
			for (int j = bar[i] + 1; j < bar[i + 1]; ++j) {
				if (shows[j] == false)
					if (++cnt3 == cnt2) {
						mdc = j;
						break;
					}
			}
			card[cnt] = mdc;
			prob[cnt] = (double) (pre_sum[bar[i + 1] - 1] - pre_sum[bar[i]]) / tot_cards;
			++cnt;
		}
		
		return cnt;
	}
	double search(int depth, Env *env) {
		double ret = -1e10;
		if (depth == maxdepth  ||  num_handcards == 0) {
			ret = env->scores[1] - env->scores[0];
			for (int i = 0; i < num_handcards; ++i)
				ret -= evaluate(env, handcards[i], i == num_handcards - 1 ? NUM_CARDS + 1 : handcards[i + 1]) / num_handcards;
			return ret;
		}
		Env *temp = stor_env + depth;
		int show, card;
		int *segs_card = stor_segs_card[depth], r_card;
		double *segs_prob = stor_segs_prob[depth], prob;
		int num_segs = gen_segs_to(segs_card, segs_prob, env);

		for (int i = 0; i < num_handcards; ++i) {
			double e = 0;
			card = handcards[i];
			for (int j = i; j < num_handcards; ++j)
				handcards[j] = handcards[j + 1];
			num_handcards--;
			for (int j = 0; j < num_segs; ++j) {
				r_card = segs_card[j], prob = segs_prob[j];
				*temp = *env;
				temp->push2(card, r_card, stor);
				stor += (STACK_DEPTH + 1) * 2;
				/*if (r_card < card) {
					temp->push(r_card, 1, stor);
					stor += STACK_DEPTH + 1;
					temp->push(card, 0, stor);
					stor += STACK_DEPTH + 1;
				}
				else {
					temp->push(card, 0, stor);
					stor += STACK_DEPTH + 1;
					temp->push(r_card, 1, stor);
					stor += STACK_DEPTH + 1;
				}*/
				//temp->push(card, stor)

				shows[r_card] = 1, shows[card] = 1;
				e += search(depth + 1, temp) * prob;
				shows[r_card] = 0, shows[card] = 0;
				stor -= (STACK_DEPTH + 1) * 2;
			}
			num_handcards++;
			for (int j = num_handcards - 1; j > i; --j)
				handcards[j] = handcards[j - 1];
			handcards[i] = card;
			if (e > ret)
				ret = e, show = card;
		}
		return depth == 0 ? show : ret;
	}
public:
	XAgent_v001() {
		env = new Env();
		stor = stor_arr;
		for (int i = 0; i < NUM_STACKS; ++i) {
			env->stacks[i] = stor;
			stor += STACK_DEPTH + 1;
		}
		
		log_fac[0] = 0;
		for (int i = 1; i <= NUM_CARDS; ++i) {
			log_fac[i] = log_fac[i - 1] + log(i);
		}
		for (int i = 0; i <= NUM_CARDS; ++i) {
			for (int j = 0; j <= NUM_CARDS; ++j)
				log_c[i][j] = log_fac[i] - log_fac[i - j] - log_fac[j];
		}
		for (int i = 1; i <= NUM_ROUNDS; ++i) {
			for (int j = 0; j <= NUM_CARDS; ++j) {
				int rest = NUM_CARDS - NUM_STACKS - (NUM_ROUNDS - i) * NUM_PLAYERS - i;
				prob_all[i][j] = j < i ? 0 : min(1., exp(log_c[j][i] - log_c[rest][i]));
				prob_single[i][j] = rest - j < i ? 1 : 1 - exp(log_c[rest - j][i] - log_c[rest][i]);
			}
		}
	}
	void Init(int pid, int rd, vector <int> __handcards, int num_players, int num_cards, vector <int> __cards, bool small_mode) {
		memset(shows, 0, sizeof(shows));
		memset(pre_sum, 0, sizeof(pre_sum));
		int card, cid = 0;
		num_handcards = NUM_ROUNDS - rd;
		for (int i = 0; i < num_handcards; ++i) {
			handcards[i] = __handcards[i];
			shows[handcards[i]] = 1;
		}
		sort(handcards, handcards + num_handcards);
			
		rd -= small_mode;
		for (int i = 0; i <= rd; ++i) {
			for (int j = 0; j < NUM_STACKS; ++j) {
				env->stacks[j][0] = 0;
				env->nimmts[j] = 0;
				for (int k = 0; k < STACK_DEPTH; ++k) {
					card = __cards[cid++];
					shows[card] = 1;
					env->stacks[j][k + 1] = card;
					env->stacks[j][0] += (card != 0);
					env->nimmts[j] += NIMMTS[card];
				}
			}
			if ((i != rd) || (small_mode == 1)) {
				for (int j = 0; j < num_players; ++j) {
					ppp[j] = card = __cards[cid++];
					shows[card] = 1;
				}
			}
		}
		tot_cards = 0;
		for (int i = 1; i <= NUM_CARDS; ++i) {
			pre_sum[i] = pre_sum[i - 1] + (shows[i] == 0);
			tot_cards += shows[i] == 0;
		}
		
		maxdepth = num_handcards;
		if (num_handcards >= 5)
			maxdepth = 3;
		if (num_handcards >= 7)
			maxdepth = 2;
		
		maxdepth = 1;
		//-------------------------------------
	}
	int policy(int pid, int rd, vector <int> handcards, int num_players, int num_cards, vector <int> cards, vector <int> scores) {
		Init(pid, rd, handcards, num_players, num_cards,cards, 0);
		int ret = (int) (search(0, env) + 1e-5);
		return ret;
	}
	int policy_min(int pid, int rd, vector <int> handcards, int num_players, int num_cards, vector <int> cards, vector <int> scores) {
		Init(pid, rd, handcards, num_players, num_cards, cards, 1);
		Env *temp = stor_env;
		double max_e = -1e10;
		int sel = -1;
		for (int i = 0; i < NUM_STACKS; ++i) {
			*temp = *env;
			temp->push(ppp[pid], 0, stor, i), stor += (STACK_DEPTH + 1);
			temp->push(ppp[1 - pid], 1, stor), stor += (STACK_DEPTH + 1);
			temp->sort();
			double e = search(1, temp);
			if (e > max_e)
				max_e = e, sel = i;
			stor -= (STACK_DEPTH + 1) * 2;
		}
		return sel;
	}
};

AgentsRegister XAgentRegister("x", new XAgent_v001());
/*
int main(int argc, char **argv) {
	NaiveAgent1v1 *agent = new NaiveAgent1v1();
}
*/



