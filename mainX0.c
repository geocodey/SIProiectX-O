#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_LINE 19
#define MAX_COLUMN 25

/* Global vars: 2 matrixes for the tables */
int game_table[MAX_LINE][MAX_COLUMN], positions_table[3][3];

/* send a char on the terminal TeraTerm */
void send_char(unsigned char c) {
	// TDRE = Transmit Data Register Empty -> has to be 1
	// TDRE & 0x80 -> we use it to check the flag and when it's ready => send the char
	while(!((SCI0SR1 & 1<<7) != 0));
	SCI0DRL = c;
}

/* send a string on the terminal TeraTerm */
void send_string(char v[]) {
    int i;
    send_char('\n');
    /* send 13 = Carriage return = \r: back at the beginning of the current line */
    send_char(13);
    for (i = 0; i < strlen(v); i++) {
        send_char(v[i]);
    }
}

/* send a string on the terminal TeraTerm, with newline */
void send_string_with_newline(char v[]) {
    send_string(v);
    send_char('\n');
}

/* send newline and \r */
void send_alignment() {
	send_char('\n'); 
    send_char(13);
}

/* read a char from TeraTerm and receive it on Dragon12 */
unsigned char read_char() {
    unsigned char c;
	// SCI0SR1 & 0x20 = SCI0SR1 & 0010 0000 -> RDRF = 1
    while(!((SCI0SR1 & 1<<5) != 0));
    if ((SCI0SR1 & 1<<5) != 0) {
       c = SCI0DRL;
       send_char(c);
       send_char(13);
    }
    return c; 
}

/* Show the rules of the game */
void game_rules() {
	send_alignment();
    send_string("X & 0 (Tic Tac Toe)\n");
    send_string("--------------------------------------------\n");
    send_string("\n");
    send_string("1. The player chooses the symbol to play with: X or 0.");
    send_string(" The computer will take the opposite one.\n");
    send_string("2. The player sets a position between 1-9: the place to put the symbol.\n");
    send_string("3. The computer makes his move.\n");
    send_string("4. Players alternate placing Xs and 0s on the game board until either opponent has three in a row or all nine squares are filled.\n");
    send_string("5. In the event that no one has three in a row, column or diagonal, it's called a draw.\n");
    send_string("\n");
}

/* 
	initialize the game table: 
	0 = ' ' 
	1 = '-'
	2 = "+'"
	3 = '|' 
*/
void initialize_game_table() {
    int i, j;
    for (i = 0; i < MAX_LINE; i++) {
        for (j = 0; j < MAX_COLUMN; j++) {
            game_table[i][j] = 0;
		}
	}
    for (i = 0; i < MAX_LINE; i++) {
        for (j = 0; j < MAX_COLUMN; j++) {
            if (i == 0 || i == 6 || i == 12 || i == 18) {
                if(j != 0 && j != 8 && j != 16 && j != 24) {
                    game_table[i][j] = 1;
				} else {
                    game_table[i][j] = 2;
				}					
            } else {
                if(j == 0 || j == 8 || j == 16 || j == 24) {
                    game_table[i][j] = 3; 
				}
			}
        }
    }
}

/* show with special characters the game table matrix */
/* 
	19 x 25 matrix with 9 positions will look like:
	
		+-------+-------+-------+
		|		|		|		|
		|		|		|		|
		|	1	|	2	|	3	|
		|		|		|		|
		|		|		|		|
		+-------+-------+-------+
		|		|		|		|
		|		|		|		|
		|	4	|	5	|	6	|
		|		|		|		|
		|		|		|		|
		+-------+-------+-------+
		|		|		|		|
		|		|		|		|
		|	7	|	8	|	9	|
		|		|		|		|
		|		|		|		|
		+-------+-------+-------+
		
*/
void show_game_table() {
    int i, j;
    send_alignment();
    for (i = 0; i < MAX_LINE; i++) {
        for (j = 0; j < MAX_COLUMN; j++) {
            switch(game_table[i][j]) {
                case 0: 
    				send_char(' '); 
                    break;
                case 1: 
    				send_char('-'); 
                    break;
                case 2: 
    				send_char('+'); 
                    break;
                case 3: 
    				send_char('|'); 
					break;
                case 4: 
    				send_char('\\'); 
				    break;
                case 5: 
					send_char('/'); 
					break;
                case 6: 
				    send_char('X'); 
					break;
                default: 
					send_alignment();
					break;
            }         
		}
		send_alignment();
    }
	send_alignment();
}

/* complete the game_table with X */
/*
	X symbol will look like:
		+-------+
		|		|
		|  \ /	|
		|	X	|
		|  / \  |
		|		|
		+-------+
*/
void set_symbol_x(int i, int j) {
	/* 
		i and j can have any value from 1 to 3
		we need to shift the position in the char game_table 
	*/
    if (i == 1) {
        i = 2; // first row cell, skip first |
	} else {
        if (i == 2) {
             i = 8; // go to second row cell, position 8
		} else {
            i = 14; // go to third row cell
		}
	}
    if (j == 1) {
        j = 3; // first column cell, skip +- and start from the third
	} else {
        if (j == 2) {
             j = 11; // go to second column cell
		} else {
            j = MAX_LINE; // go to third column cell
		}
	}
    game_table[i][j] = game_table[i + 2][j + 2] = 4; // put '\'
    game_table[i + 1][j + 1] = 6; // put 'X'
    game_table[i][j + 2] = game_table[i + 2][j] = 5; // put '/'
}

/* complete the game_table with 0 */
/*
	0 symbol will look like:
		+-------+
		|		|
		| /---\	|
		| |	  | |
		| \---/ |
		|		|
		+-------+
*/
void set_symbol_0(int i, int j ) {
    if (i == 1) {
        i = 2;
	} else {
        if (i == 2) {
             i = 8;
		} else {
            i = 14;
		}
	}
    if (j == 1) {
        j = 2;
	} else {
        if (j == 2) {
             j = 10;
		} else {
            j = 18;
		}
	}
    game_table[i + 2][j] = game_table[i][j + 4] = 4; // put '\'
    game_table[i][j + 1] = game_table[i][j + 2] = game_table[i][j + 3] = game_table[i + 2][j + 1] = game_table[i + 2][j + 2] = game_table[i + 2][j + 3]= 1; // put '-'
    game_table[i][j] = game_table[i + 2][j + 4] = 5; // put '/'
    game_table[i + 1][j] = game_table[i + 1][j + 4] = 3; // put '|'
}

/* 
	we codify the positions_table with 2 for player X and 3 for player 0.
	0 means a free position.
 */
int game_matrix() {
    int winner = 0;
    /* check if player won, if true -> return 2, else, return 3 for the computer */
	if ((positions_table[0][0] == 2 && positions_table[1][1] == 2 && positions_table[2][2] == 2) || (positions_table[0][2] == 2 && positions_table[1][1] == 2 && positions_table[2][0] == 2) ||
		 (positions_table[0][1] == 2 && positions_table[1][1] == 2 && positions_table[2][1] == 2) || (positions_table[1][0] == 2 && positions_table[1][1] == 2 && positions_table[1][2] == 2) ||
		 (positions_table[0][0] == 2 && positions_table[0][1] == 2 && positions_table[0][2] == 2) || (positions_table[2][0] == 2 && positions_table[2][1] == 2 && positions_table[2][2] == 2) ||
		 (positions_table[0][0] == 2 && positions_table[1][0] == 2 && positions_table[2][0] == 2) || (positions_table[0][2] == 2 && positions_table[1][2] == 2 && positions_table[2][2] == 2))
			winner = 2;
	else {
		if ((positions_table[0][0] == 3 && positions_table[1][1] == 3 && positions_table[2][2] == 3) || (positions_table[0][2] == 3 && positions_table[1][1] == 3 && positions_table[2][0] == 3) ||
			 (positions_table[0][1] == 3 && positions_table[1][1] == 3 && positions_table[2][1] == 3) || (positions_table[1][0] == 3 && positions_table[1][1] == 3 && positions_table[1][2] == 3) ||
			 (positions_table[0][0] == 3 && positions_table[0][1] == 3 && positions_table[0][2] == 3) || (positions_table[2][0] == 3 && positions_table[2][1] == 3 && positions_table[2][2] == 3) ||
			 (positions_table[0][0] == 3 && positions_table[1][0] == 3 && positions_table[2][0] == 3) || (positions_table[0][2] == 3 && positions_table[1][2] == 3 && positions_table[2][2] == 3))
					winner = 3;
        }
    return winner;
}

/* check for draw between opponents. If positions_table is full and nobody won so far. */
int it_is_draw() {
    int i, j, count = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (positions_table[i][j] != 0) {
                count++;
			}
		}
	}
    if (count == 9) {
        return 1;
	}
    return 0;
}

int check_diag(int i, int j) {
    if ((i == 0 && j == 0) || (i == 0 && j == 2) || (i == 2 && j == 0) || (i == 2 && j == 2)
        || (i == 1 && j == 1)) {
        return 1;
    }
    return 0;
}

int check_borders(int i, int j) {
    if ((i >= 0 && j >= 0) && (i < 3 && j < 3)){
       return 1; 
    }
    return 0;
}

  unsigned short lfsr = 0xACE1u;
  unsigned bit;

  unsigned rnd()
  {
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    return lfsr =  (lfsr >> 1) | (bit << 15);
  }

/* select a number out of an interval for the next move of Dragon12 */
int ai_for_board() {

	int i, j;
    
    for(i = 0; i < 3; i++){
        if(!positions_table[i][0] && positions_table[i][1] && (positions_table[i][1] == positions_table[i][2]))
            return i*10 + 0;
        if(!positions_table[i][1] && positions_table[i][0] && (positions_table[i][0] == positions_table[i][2]))
            return i*10 + 1;
        if(!positions_table[i][2] && positions_table[i][0] && (positions_table[i][0] == positions_table[i][1]))
            return i*10 + 2;
    }
    for(i = 0; i < 3; i++){
        if(!positions_table[0][i] && positions_table[1][i] && (positions_table[1][i] == positions_table[2][i]))
            return 0*10 + i;
        if(!positions_table[1][i] && positions_table[1][i] && (positions_table[0][i] == positions_table[2][i]))
            return 1*10 + i;
        if(!positions_table[2][i] && positions_table[1][i] && (positions_table[0][i] == positions_table[1][i]))
            return 2*10 + i;
    }
    for(i = 0; i < 3; i++){
        for(j = 0; j < 3; j++){
            if(!positions_table[i][j])
                return i*10 + j;
        }
    }
	
}

/* send 1 if the game has a winner, or it's a draw, or 0 if the game needs to continue */
int check_for_winner(char user_symbol, char opponent_symbol) {
    /* show the matrix after every move */
    show_game_table();
    
    /* check for game status: who won or if it's a draw */
    if (game_matrix() == 3) {
        send_string_with_newline("Game over! Winner is player: ");
        send_char(opponent_symbol);
        send_string("\n");
		return 1;
    } else {
        if (game_matrix() == 2) {
            send_string_with_newline("Game over! Winner is player: ");
            send_char(user_symbol);
            send_string("\n");
			return 1;
        } else if (it_is_draw() == 1) {
            send_string("Game over! It is a draw!\n");
			return 1;
        }
    }
	return 0;
}

void main(void) {
    int player_digit;
	  int i, j, m, n;
    int position, position0;
    char position_on_table;
    char user_symbol, opponent_symbol;
    char enter;
	
	/* 
		We set the registers.
		See documentation -> page 14
	*/
	SCI0BDH = 0x00;
	SCI0BDL = 0x1A; // 0001 1010: SCI baud rate = 26 
	SCI0CR1 = 0x00;
	SCI0CR2 = 0x0C; // 0000 1100: Transmitter Enabled + Receiver Enabled 
	
	game_rules();
	
	/* Loop for every game */
    LOOP: {
		send_string("New game!\n");
		
		/* Initialize the matrix -> 0 everywhere */
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				positions_table[i][j] = 0;
			}
		}

		/* initialize and show the game table matrix */
		initialize_game_table();
		show_game_table();
		
		send_string("Insert the symbol you want to play with: X or 0 (use the digit 0)\n");
		do {
			user_symbol = read_char();
			enter = read_char();
			user_symbol = toupper(user_symbol);
			if (user_symbol != 'X' && user_symbol != '0') {
				send_string("Wrong symbol! Try again: with X or 0\n");      
			}
		} while (user_symbol != 'X' && user_symbol != '0');
		 
		while (1) {
			send_string_with_newline("Insert the position in the game table: ");
			do {
				position_on_table = read_char();
				enter = read_char();
				position = position_on_table - '0';
				
				// put indexes in positions_table matrix from 1 to 3
				switch (position) {
					case 1:
						i = 1; j = 1;
						break;
					case 2:
						i = 1; j = 2;
						break;
					case 3:
						i = 1; j = 3;
						break;
					case 4:
						i = 2; j = 1;
						break;
					case 5: 
						i = 2; j = 2;
						break;
					case 6:
						i = 2; j = 3;
						break;
					case 7: 
						i = 3; j = 1;
						break;
					case 8:
						i = 3; j = 2;
						break;
					case 9:
						i = 3; j = 3;
						break;
					default:
						i = 0; j = 0;
						break;
				}
				if ((i < 1) || (i > 3) || (j < 1) || (j > 3) || (positions_table[i - 1][j - 1] != 0)) { // wrong number or occupied position already
					send_string("Wrong position! Try again!\n");
				}
			} while ((i < 1) || (i > 3) || (j < 1) || (j > 3) || (positions_table[i - 1][j - 1] != 0));
			
			/* if the position is ok, set the positions_table matrix with the value 2 */
			positions_table[i - 1][j - 1] = 2;
			
			/* set the players' symbol for the current game depending on what the user specifies: X or 0 */
			if (user_symbol == 'X') { // show on game_table the symbol X and set opponent's symbol 0
			  set_symbol_x(i, j); 
			  opponent_symbol = '0';
			  player_digit = 2;
			} else {
				set_symbol_0(i, j);
				opponent_symbol = 'X';
				player_digit = 3;
			}
			
			if (check_for_winner(user_symbol, opponent_symbol)) {
				goto LOOP;
			}
					   
			send_string("Dragon12 Board\n");
			do {
				position0 = ai_for_board();
				switch (position0) {
					case 0:
						m = 1; n = 1;
						break;
					case 1:
						m = 1; n = 2;
						break;
					case 2:
						m = 1; n = 3;
						break;
					case 10: 
						m = 2; n = 1;
						break;
					case 11:
						m = 2; n = 2;
						break;
					case 12:
						m = 2; n = 3;
						break;
					case 20: 
						m = 3; n = 1;
						break;
					case 21:
						m = 3; n = 2;
						break;
					case 22:
						m = 3; n = 3;
						break;
					default:
						m = 0; n = 0;
						break;
				}
			} while ((m < 1) || (m > 3) || (n < 1) || (n > 3) || (positions_table[m - 1][n - 1] != 0));
			
			positions_table[m - 1][n - 1] = 3;
			if (opponent_symbol == '0') {
				set_symbol_0(m, n);
			} else {
				set_symbol_x(m, n);
			}
			
			if (check_for_winner(user_symbol, opponent_symbol)) {
				goto LOOP;
			}
		}
    }   
}
