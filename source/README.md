./main.out [PATH] [PATH] … [-cgs number]

# OPTIONS:

PATH			the PATH of the program
-c number		Set the number of cards, default 104
-g number		Set the number of games, default 1
-s 0|1			s=0 means long game (stop until a player gets -66), s=1 means short game (a single game), default s=1

# EXAMPLE:
./main.out ./trivial.out ./trivial.out ./easiest2.out -c 24 -s 0 -g 100

--There are 3 players, two with the program trivial.out, one with the program easiest2.out, the cards are 1 to 24, and there are 100 long games.

# PLAYERS PROGRAM
./filename [-small]

-small			:SMALL mode, otherwise NORMAL mode

for NORMAL mode, the player selects one card to play, so the output is the card
for SMALL mode, the player's card is the smallest among all players, and it is smaller than the last card in every line on table, so the player need to select a line to replace. The output is the index of the line (0 to 3). 

# INPUT FORMAT
A player’s program only received standard input stdin, so you can use scanf or cin to get the input numbers. The format of the input:

## Private information:
--the first line has two numbers: the index of this player, and the index of current round, which are from 0 to 9
--the second line shows the cards in hand of this player, so there are 10-current_round numbers.

## Public information:
--the third line has two numbers: the total number of players and the total number of cards;
—-there are still 5*current_round+4 lines in NORMAL mode, and 5*(current_round-1) lines in SMALL mode. That is, when deciding which lines to replace, it already enters the next round.
--the next 4 lines are the cards on table initially, every line has 5 numbers, with 0s filled for empty slots.
--the next line are the cards played (ordered from player 0) in round 0
--the next 4 lines are the cards on table after round 0
--repeat this until the current round, so the program only append more lines each time
--in SMALL mode, the last line is the cards played, otherwise the last 4 lines shows the cards on table. 


# OUTPUT FORMAT:
The output is only one number, which can be output by standard method stdout (using printf or cout), or can be stored in "output.txt" (the main program will delete it every time to prevent other players seeing it). The “output.txt” should only be used if you cannot manage the running time.

# GRADING:
In a 2-player game, the winner of each game will receive 1 point, the loser 0 point. In case of draw both players will receive 0.5 point.

In a multi-player game, the player with the highest score will receive 1 point; in case of a tie of k players with the highest score, each of these k players will receive 1/k point. Similarly, the k' players with the lowest score will receive -1/k point each.


"log.txt"
--the log for all games played
--in order to avoid too large log, you can delete it by "rm log.txt"