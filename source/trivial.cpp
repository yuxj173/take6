#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
using namespace std;


#define MAX_CARDS 104
#define MAX_PLAYERS 10
#define NUM_ROUNDS 10
#define NUM_STACKS 4
#define STACK 5



#define TIME_LIMIT 2


int player_index;
int current_round;

int main(int argc, char *argv[]) {
  srand(clock());
  int i;
   
  int cards[NUM_ROUNDS];
  cin >> player_index;
  
  cin  >> current_round;

  for (i=0;i<NUM_ROUNDS-current_round;i++)
    cin >> cards[i];
      
   
  
  if ((argc==2) &&  (argv[1][1]=='s'))
    cout << (rand()% NUM_STACKS) << endl;
      
  
  else 
    cout << cards[rand()%(NUM_ROUNDS-current_round)] << endl;


  return 0;
 
}

