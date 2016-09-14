#include "agent.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#define NUM_PLAYERS 2
using namespace std;

class ZAgent_v001 : public Agent {
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
	double prob_c[20][110][20];
	double t_prob[20][110][20];
	double prob_double[20][110];
	int ppp[20];
	double stor_e[20][20][20];
	int scr[20], sc[20];
	int th_single[20];
	
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
	inline double prob(const int &a, const int &b, const int &c) {
		return prob_c[num_handcards][pre_sum[b - 1] - pre_sum[a]][c];
	}
	double ex_p(Env *env) {
		env->sort();
		double ret = 0;
		int mini_p = env->nimmts[env->min_id()];
		int my_free = 0, num_free = 0;
		double expro = 0;
		int id = num_handcards;
		int threshold = th_single[num_handcards];
		for (int i = NUM_STACKS; i--; ) { 
			int *s = env->stacks[i];
			int j, k = id, nex = i == NUM_STACKS - 1 ? NUM_CARDS + 1 : env->stacks[i + 1][env->stacks[i + 1][0]];
			while (id > 0  &&  handcards[id - 1] > env->stacks[i][env->stacks[i][0]])
				--id;
			j = id;
			
			if (j != k  &&  s[0] < 5)
				my_free ++;
			if (s[0] < 5  &&  nex > s[s[0]] + threshold)
				num_free ++;
		}
		id = num_handcards;
		num_free = max(num_free, 1);
		for (int i = NUM_STACKS; i--; ) {
			int *s = env->stacks[i];	
			int j, k = id, nex = i == NUM_STACKS - 1 ? NUM_CARDS + 1 : env->stacks[i + 1][env->stacks[i + 1][0]];
			while (id > 0  &&  handcards[id - 1] > env->stacks[i][env->stacks[i][0]])
				--id;
			j = id;
			if (s[0] == 5) {
				if (j == k) {
					ret -= prob(s[s[0]], nex, 1) * env->nimmts[i];
					if (expro > 1e-5)
						ret += (1 - prob(s[s[0]], handcards[k], 1)) * env->nimmts[i] * expro / 2;
				}
				else if (j < k) {
					ret += (1 - prob(s[s[0]], handcards[j], 1) / (k - j + expro)) * env->nimmts[i];
					mini_p = min(mini_p, handcards[j] + 2);
				}
				expro = j < k - 1 ? 0.3 : 0;
			}
			if (s[0] == 4) {
				if (j < k) {
					ret += prob(s[s[0]], handcards[k - 1], 1) * (env->nimmts[i] + 2);
					mini_p = min(mini_p, handcards[j] + 2);
				}
				if (j == k - 1) {
					ret -= prob(handcards[k - 1], nex, 1) * env->nimmts[i];
					if (expro > 1e-5)
                       ret += prob(s[s[0]], handcards[k], 1) * (env->nimmts[i] + 2) * expro / 2;
				}
				expro = j < k - 1 ? 0.5 : 0;
			}
			if (s[0] <= 3) {
				if ((k - j) * 2 + s[0] > STACK_DEPTH) {
					int r = s[0] - (k - j);
					if (r > 0)
						ret += prob(s[s[0]], handcards[k - 1], r) * env->nimmts[i] / (5 - s[0]);
					else 
						ret += prob(handcards[k - 1 - (STACK_DEPTH - s[0]) / 2], handcards[k - 1], 1) * env->nimmts[i] / (5 - s[0]);
					ret -= prob(handcards[k - 1], nex, 1) * env->nimmts[i] / (5 - s[0]);
				}
				if (k - j >= 2)
					expro = (s[0] - (k - j) < 0 ? prob(handcards[j], nex, 1) : 1) * (1 - 1. / my_free);
				else  if (k - j >= 1)
					expro = max(prob(handcards[j], nex, 1) - prob(s[s[0]], handcards[j], 1), 0.) * (1 - 1. / my_free);
			}
		}
		ret += (1 - prob(0, handcards[0], 1)) * mini_p * (id != 0 ? 1 : expro);
		for (int i = 1; i < id; ++i) {
			if (prob(handcards[i], handcards[i - 1], 1) < 0.3)
				ret += (1 - prob(handcards[i], handcards[i - 1], 1)) * min(NIMMTS[handcards[i]], NIMMTS[handcards[i - 1]]) * (my_free == num_free ? 1 : 0.5); 
		}
		return ret;
	}
	double search(int depth, Env *env) {
		double ret = -1e10;
		if (depth == maxdepth  ||  num_handcards == 0) {
			ret = env->scores[1] - env->scores[0];
			return ret - (num_handcards == 0 ? 0 : ex_p(env));
		}
		Env *temp = stor_env + depth;
		int show, card;
		int *segs_card = stor_segs_card[depth], r_card;
		double *segs_prob = stor_segs_prob[depth], prob;
		int num_segs = gen_segs_to(segs_card, segs_prob, env);
		double (*se)[20] = stor_e[depth];

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
				shows[r_card] = 1, shows[card] = 1;
				
				scr[depth] = r_card;
				sc[depth] = card;
				
				if (depth + 1 < maxdepth  &&  num_handcards > 0) {
					tot_cards = 0;
					
					for (int i = 1; i <= NUM_CARDS; ++i) {
						pre_sum[i] = pre_sum[i - 1] + (shows[i] == 0);
						tot_cards += shows[i] == 0;
					}
				}
				
				se[i][j] = search(depth + 1, temp);
				e += se[i][j]; //* prob;
				
				shows[r_card] = 0, shows[card] = 0;
				if (depth + 1 < maxdepth  &&  num_handcards > 0) {
					tot_cards = 0;
					for (int i = 1; i <= NUM_CARDS; ++i) {
						pre_sum[i] = pre_sum[i - 1] + (shows[i] == 0);
						tot_cards += shows[i] == 0;
					}
				}
				
				stor -= (STACK_DEPTH + 1) * 2;
			}
			num_handcards++;
			for (int j = num_handcards - 1; j > i; --j)
				handcards[j] = handcards[j - 1];
			handcards[i] = card;
			if (e > ret)
				ret = e, show = card;
		}
		return depth == 0 ? show : ret / num_segs;
	}
public:
	ZAgent_v001() {
		env = new Env();
		stor = stor_arr;
		for (int i = 0; i < NUM_STACKS; ++i) {
			env->stacks[i] = stor;
			stor += STACK_DEPTH + 1;
		}
		memset(t_prob, 0, sizeof(t_prob));
		memset(prob_c, 0, sizeof(prob_c));
		
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
				for (int k = i; k >= 0; --k) {
					int rest = NUM_CARDS - NUM_STACKS - (NUM_ROUNDS - i) * NUM_PLAYERS - i;
					t_prob[i][j][k] = (j < k  ||  rest - j < i - k) ? 0 : max(min(1., exp(log_c[rest - j][i - k] + log_c[j][k] - log_c[rest][i])), 0.);
					prob_c[i][j][k] = k == i ? t_prob[i][j][k] : prob_c[i][j][k + 1] + t_prob[i][j][k];
				}
			}
		}
		for (int i = 1; i <= NUM_ROUNDS; ++i) {
			for (int j = 0; j <= NUM_CARDS; ++j)
				if (prob_c[i][j][1] >= 0.6) {
					th_single[i] = j;
					break;
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

AgentsRegister X2AgentRegister("z", new ZAgent_v001());
/*
int main(int argc, char **argv) {
	NaiveAgent1v1 *agent = new NaiveAgent1v1();
}
*/



