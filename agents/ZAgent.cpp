/*
	agent : ZAgent
	version : sp
	author : YuXJ173
	level : easy
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

double *ZAgentRkCmp_Arr;

bool ZAgentRkCmp(const int &a, const int &b) {
	return ZAgentRkCmp_Arr[a] < ZAgentRkCmp_Arr[b];
}

class ZAgent_sp: public Agent {
	bool *rk_cmp(const int &a, const int &b);
	double **rk_cmp_arr;
	
	int handcards[10], num_handcards;
	int psum_cards[110], tot_cards;
	int stor_min_id_nimmts;
	int bar[20];
	bool shows[110];
	int _;
	
	int stor_arr[1010], *stor;
	int stor_pred_mycards_len[20][20];
	int stor_pred_segs_len[20][20];
	int stor_pred_segs_cards[20][20];
	int stor_rk[20][20];
	int stor_rrk[20][20];
	int ppp[20];
	double stor_v[20][20][20];
	double stor_e[20][20];
	double stor_ret_e[20][20];
	double stor_re[20][20];
	double log_c[110][110], log_fac[110];
	double prob_single[20][110];
	double prob_all[20][110];
	
	int threshold[10], start[10], end[10];
	int maxdepth;
	
	struct Env {
		int *stacks[NUM_STACKS], nimmts[NUM_STACKS], scores[NUM_PLAYERS];

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
				for (int j = i + 1; j < NUM_STACKS; ++j)
					if (stacks[j - 1][stacks[j - 1][0]] > stacks[j][stacks[j][0]])
						swap(stacks[j], stacks[j - 1]), swap(nimmts[j], nimmts[j - 1]);
		}
		inline int min_id() {
			return min_element(nimmts, nimmts + NUM_STACKS) - nimmts;
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
				else if (x != s2  &&  (s2 != 0  &&  stacks[s2 - 1][0] == 5)) {
					if (stacks[s2][0] != 5) {
						if (nimmts[x] > nimmts[s2] - nimmts[s2 - 1]) x = s2;
					}
					else {
						if (nimmts[x] - nimmts[s2] > nimmts[s2] - nimmts[s2 - 1]) x = s2;
					}
				}
				push(card1, 0, stor, x);
				push(card2, 1, stor);
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
				push(card1, 0, stor);
			}
			else if (card1 < card2)
				push(card1, 0, stor), push(card2, 1, stor);
			else push(card2, 1, stor), push(card1, 0, stor);
		}
	};
	Env *env, stor_env[20];
	
	inline int evaluate_threshold(const int &cnt, const int &nimmts) {
		if (cnt == 5) return nimmts <= 7 ? 4 : 2;
		else if (cnt == 4) return nimmts <= 5 ? 4 : 1;
		else if (cnt == 1) return 3;
		else return 2;
	}
	inline double prob_all_in(const int &num_handcards, const int &len) {
		return prob_all[num_handcards][len];
	}
	inline double my_prob(const int &a, const int &b) {
		return prob_single[num_handcards][psum_cards[b - 1] - psum_cards[a] + 1];
	}
	inline double rival_prob(const int &a, const int &b) {
		return prob_single[num_handcards][psum_cards[b - 1] - psum_cards[a]];
	}
	double evaluate(Env *env) {
		env->sort();
		
		double pay = 0, pro = 0;
		int cnt = 0, id = num_handcards;
		int sel = false;
		int sel_card, sel_base;
		double sel_prob;
		int sel_punish = env->nimmts[env->min_id()];
		
		for (int i = NUM_STACKS; i--; ) {
			threshold[i] = evaluate_threshold(env->stacks[i][0], env->nimmts[i]); 
			start[i] = id;
			while (id > 0  &&  handcards[id - 1] > env->stacks[i][env->stacks[i][0]]) 
				if (start[i] - (--id) > threshold[i]) break;
			end[i] = id;
		}
		
		int extra = 4 - id;
		int current_four_threshold;
		for (int i = 0; i <= NUM_CARDS; ++i) {
			if (prob_single[num_handcards][i] <= 0.3)
				current_four_threshold = i;
			else
				break;
		}
		
		for (int i = 0; i < NUM_STACKS; ++i) {
		
			int r = i < NUM_STACKS - 1 ? env->stacks[i + 1][0] : NUM_CARDS + 1;
			while (extra > 0  &&  end[i] < start[i]  &&  handcards[end[i] + 1] < r)
				++end[i], --extra;
			if (i == NUM_STACKS - 1)
				break;
			if (env->stacks[i][0] == 5) 
				extra = start[i] == end[i] ? 0 : 3 - (start[i] - end[i]);
			else if (env->stacks[i][0] == 4) {
				if (start[i] - end[i] >= 2) extra = 2;
				else extra = handcards[end[i + 1]] < env->stacks[i][env->stacks[i][0]] + current_four_threshold;
			}
			else extra = threshold[i] - (start[i] - end[i]);
			if (i != 0)
				start[i - 1] = end[i];
		}
		
		if (end[0] > 0) {
			sel = true;
			for (int i = end[0] - 1; i >= 0; --i)
				if (NIMMTS[handcards[i]] == 1  ||  i == 0) {
					sel_card = handcards[i];
					sel_base = i != 0;
					break;
				}
			sel_prob = (1 - rival_prob(0, sel_card));
		}
		
		double rival_sel_prob = rival_prob(0, env->stacks[0][env->stacks[0][0]]);
		int num_free_stacks = 0;
		for (int i = 0; i < NUM_STACKS; ++i) {
			if (env->stacks[i][0] <= 4  &&  start[i] != end[i])
				num_free_stacks++;
		}
		
		int last_punish = 0, head, k;
		for (int i = 0; i < NUM_STACKS; ++i) {
			int r = i < NUM_STACKS - 1 ? env->stacks[i + 1][0] : NUM_CARDS + 1;
			head = env->stacks[i][env->stacks[i][0]];
			k = start[i] - end[i];
			if (env->stacks[i][0] == 5) {
				if (k == 0) pay -= rival_prob(head, r) * env->nimmts[i], last_punish = 1;
				else {
					if (k == 1) pay += (1 - rival_prob(head, handcards[end[i]])) * env->nimmts[i];
					else {
						pay += (1 - rival_prob(handcards[end[i]], handcards[end[i] + 1])) * env->nimmts[i] * 0.5;
						pay += (1 - rival_prob(head, handcards[end[i]])) * env->nimmts[i] * 0.5;
					}
					sel_punish = min(sel_punish, NIMMTS[handcards[end[i]]] + (k == 1 ? 0 : NIMMTS[handcards[end[i] + 1]]));
				}
			}
			else if (env->stacks[i][0] == 4) {
				if (k == 1) {
					pay += rival_prob(head, handcards[end[i]]) * (env->nimmts[i] + 1.5);
					pay -= rival_prob(handcards[end[i]], k == 2 ? handcards[end[i] + 1] : r) * (env->nimmts[i] + 1.5);
					sel_punish = min(sel_punish, NIMMTS[handcards[end[i]]] + (k == 1 ? 0 : NIMMTS[handcards[end[i] + 1]]));
				}
			}
			else if (k) sel_prob *= 0.8;
			if (last_punish  &&  env->nimmts[i - 1] > env->nimmts[i]  &&  num_free_stacks) {
				if (k >= 2) {
					if (env->stacks[i - 1][0] == 4)
						pay += max(0., (rival_prob(env->stacks[i - 1][env->stacks[i - 1][0]], handcards[end[i] + 1]) * env->nimmts[i - 1] - env->nimmts[i]) * (rival_sel_prob / NUM_STACKS + rival_prob(handcards[end[i] + 1], r)) * (1 - 1. / (num_free_stacks + 1)));
					}
				if (k >= 1) {
					if (env->stacks[i - 1][0] == 4)
						pay += max(0., (rival_prob(env->stacks[i - 1][env->stacks[i - 1][0]], handcards[end[i]]) * env->nimmts[i - 1] - env->nimmts[i]) * (rival_sel_prob / NUM_STACKS + rival_prob(handcards[end[i]], r)) * (1 - 1. / (num_free_stacks + 1)));
					else {
						pay += max(0., ((1 - 2 * rival_prob(env->stacks[i - 1][env->stacks[i - 1][0]], handcards[end[i]])) * env->nimmts[i - 1] - env->nimmts[i]) * (rival_sel_prob / 4 + rival_prob(handcards[end[i]], r)) * (1 - 1. / (num_free_stacks + 1)));
						pay += (env->nimmts[i - 1] - env->nimmts[i]) * rival_sel_prob * (1. / num_free_stacks);
					}
				}
			}
			last_punish = env->stacks[i][0] >= 4  &&  k == 0;
		}
		
		if (sel) pay += sel_prob * (sel_punish + sel_base);
		
		if (num_handcards == 5  &&  handcards[2] == 28  &&  handcards[3] == 85) {
			//cerr << env->scores[1] - env->scores[0] << " " << pay << " " << endl;
		}
		
		return env->scores[1] - env->scores[0] - pay;
	}
	int gen_segs_to(Env *env, int *card, int *len, int *my_len) {
		int cnt = 0, num = 0;
		double maxi = 0, tot = 0;
		bar[0] = 0;
		int sid = -1;
		for (int i = 0; i < num_handcards; ++i) {
			bar[++num] = handcards[i];
			while (sid < NUM_STACKS - 1  &&  env->stacks[sid + 1][env->stacks[sid + 1][0]] < handcards[i])
				++sid;
			int last = i == 0 ? 0 : handcards[i - 1];
			int next = i == num_handcards - 1 ? NUM_CARDS + 1 : handcards[i + 1];
			if (sid >= 0  &&  env->stacks[sid][env->stacks[sid][0]] > last)
				last = env->stacks[sid][env->stacks[sid][0]];
			else
				last = i == 0 ? 0 : (handcards[i - 1] + handcards[i]) / 2;
			if (sid < NUM_STACKS - 1  &&  env->stacks[sid + 1][env->stacks[sid + 1][0]] < next)
				next = env->stacks[sid + 1][env->stacks[sid + 1][0]];
			else
				next = i == num_handcards - 1 ? NUM_CARDS + 1 : (handcards[i + 1] + handcards[i]) / 2 + 1;
			my_len[i] = psum_cards[next - 1] - psum_cards[last] + 1;
		}
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
			for (int j = bar[i] + 1; j < bar[i + 1]; ++j) {
				if (shows[j] == false)
					if (++cnt3 == 1) {
						mdc = j;
						break;
					}
			}
			card[cnt] = mdc;
			len[cnt] = cnt2;
			++cnt;
		}
		return cnt;
	}
	double search(int depth, Env *env) {
		if (num_handcards == 0)
			return env->scores[1] - env->scores[0];
		if (depth == maxdepth)
			return evaluate(env);
		
		Env *next_env = stor_env + depth;
		int *pred_segs_cards = stor_pred_segs_cards[depth];
		int *pred_segs_len = stor_pred_segs_len[depth];
		int *pred_mycards_len = stor_pred_mycards_len[depth];
		int num_pred_segs = gen_segs_to(env, pred_segs_cards, pred_segs_len, pred_mycards_len);
		int *rk = stor_rk[depth];
		int *rrk = stor_rrk[depth];
		double (*v)[20] = stor_v[depth];
		double *e = stor_e[depth];
		double *re = stor_e[depth];
		double prob = 0, next_prob;
		int len = 0;
		int r_card, card;
		for (int j = 0; j < num_pred_segs; ++j) {
			for (int i = 0; i < num_handcards; ++i) {
				r_card = pred_segs_cards[j], card = handcards[i];
				
				*next_env = *env;
				next_env->push2(card, r_card, stor);
				stor += (STACK_DEPTH + 1) * 2;
				
				num_handcards--;
				for (int k = i; k < num_handcards; ++k)
					handcards[k] = handcards[k + 1];
			
				shows[r_card] = 1, shows[card] = 1;
				
				if (num_handcards > 0  &&  depth + 1 < maxdepth) {
					for (int k = 1; k <= NUM_CARDS; ++k)
						psum_cards[k] = psum_cards[k - 1] + (shows[k] == 0);
					next_env->sort();
				}
				
				e[i] = v[i][j] = search(depth + 1, next_env);
				rk[i] = i;
				
				if (num_handcards > 0  &&  depth + 1 < maxdepth) {
					for (int k = 1; k <= NUM_CARDS; ++k)
						psum_cards[k] = psum_cards[k - 1] + (shows[k] == 0);
				}
				
				shows[r_card] = 0, shows[card] = 0;
				for (int k = num_handcards - 1; k > i; --k)
					handcards[k] = handcards[k - 1];
				handcards[i] = card;
				
				num_handcards++;
				
				stor -= (STACK_DEPTH + 1) * 2;
			}
			ZAgentRkCmp_Arr = e, sort(rk, rk + num_handcards, ZAgentRkCmp);
			prob = 0, len = 0;
			re[j] = 0;
			for (int i = 0; i < num_handcards; ++i) {
				next_prob = prob_all_in(num_handcards, len += pred_mycards_len[rk[i]]); 
				re[j] += prob_single[num_handcards][pred_mycards_len[rk[i]]] * e[rk[i]];
				prob = next_prob; 
			}
			rrk[j] = j;
		}
		ZAgentRkCmp_Arr = re, sort(rrk, rrk + num_pred_segs, ZAgentRkCmp);
		prob = 0, len = 0;
		
		double *ret_e = stor_ret_e[depth];
		for (int i = 0; i < num_handcards; ++i)
			ret_e[i] = 0;
		for (int j = num_pred_segs; j--; ) {
			next_prob = prob_all_in(num_handcards, len += pred_segs_len[rrk[j]]);
			for (int i = 0; i < num_handcards; ++i)	
				ret_e[i] += (next_prob - prob) * v[i][rrk[j]];
			prob = next_prob;
		}
		double max_e = -1e10;
		int ret_card = -1;
		for (int i = 0; i < num_handcards; ++i)
			if (ret_e[i] > max_e)
				max_e = ret_e[i], ret_card = handcards[i];
		
		if (depth == 0) {
			cerr << endl;
			for (int i = 0; i < num_handcards; ++i) 
				cerr << "[" << handcards[i] << " " << ret_e[i] << "] ";
			cerr << endl;
		}
		return depth == 0 ? ret_card : max_e;
	}
public:
	ZAgent_sp() {
		env = new Env();
		stor = stor_arr;
		for (int i = 0; i < NUM_STACKS; ++i) {
			env->stacks[i] = stor;
			stor += STACK_DEPTH + 1;
		}
	
		//--------------------------------------------

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
		memset(psum_cards, 0, sizeof(psum_cards));
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
					card = __cards[cid++];
					ppp[j] = card;
					shows[card] = 1;
				}
			}
		}
		
		for (int i = 1; i <= NUM_CARDS; ++i)
			psum_cards[i] = psum_cards[i - 1] + (shows[i] == 0);
		
		maxdepth = num_handcards;
		if (num_handcards >= 5)
			maxdepth = 3;
		if (num_handcards >= 8)
			maxdepth = 2;		
		//maxdepth = 1;
		
		//-------------------------------------
	}
	int policy(int pid, int rd, vector <int> handcards, int num_players, int num_cards, vector <int> cards, vector <int> scores) {
		Init(pid, rd, handcards, num_players, num_cards,cards, 0);
		env->sort();
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
			temp->push(ppp[pid], 0, stor, i);
			temp->push(ppp[1 - pid], 1, stor);
			stor += (STACK_DEPTH + 1) * 2;
			temp->sort();
			double e = search(1, temp);
			if (e > max_e)
				max_e = e, sel = i;
			stor -= (STACK_DEPTH + 1) * 2;
		}
		return sel;
	}
};



AgentsRegister ZAgentRegister("zsp", new ZAgent_sp());

/*
int main(int argc, char **argv) {
	NaiveAgent1v1 *agent = new NaiveAgent1v1();
}
*/
