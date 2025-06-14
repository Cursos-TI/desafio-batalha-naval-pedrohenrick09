#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define BOARD_SIZE 10
#define NUM_SHIPS 5

char player_board[BOARD_SIZE][BOARD_SIZE];
char computer_board[BOARD_SIZE][BOARD_SIZE];
char player_view[BOARD_SIZE][BOARD_SIZE];

const int ship_lengths[NUM_SHIPS] = {5, 4, 3, 3, 2};
const char* ship_names[NUM_SHIPS] = {"Porta-avioes", "Navio-tanque", "Contra-torpedeiro", "Submarino", "Patrulha"};

typedef enum { HUNT, TARGET } AI_Mode;
AI_Mode ai_mode = HUNT;
int target_row = -1, target_col = -1;
int first_hit_row = -1, first_hit_col = -1;
int last_hit_row = -1, last_hit_col = -1;
int tried_dirs[4] = {0, 0, 0, 0}; // 0:Up, 1:Down, 2:Left, 3:Right

void initialize_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '~';
        }
    }
}

void print_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    printf("\n  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", i);
    }
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%c ", 'A' + i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int is_valid(int r, int c) {
    return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
}

int is_valid_placement(char board[BOARD_SIZE][BOARD_SIZE], int row, int col, int length, char orientation) {
    if (orientation == 'H' || orientation == 'h') {
        if (col + length > BOARD_SIZE) return 0;
        for (int i = 0; i < length; i++) {
            if (board[row][col + i] != '~') return 0;
        }
    } else if (orientation == 'V' || orientation == 'v') {
        if (row + length > BOARD_SIZE) return 0;
        for (int i = 0; i < length; i++) {
            if (board[row + i][col] != '~') return 0;
        }
    } else {
        return 0;
    }
    return 1;
}

void place_ship(char board[BOARD_SIZE][BOARD_SIZE], int row, int col, int length, char orientation) {
    if (orientation == 'H' || orientation == 'h') {
        for (int i = 0; i < length; i++) board[row][col + i] = 'S';
    } else {
        for (int i = 0; i < length; i++) board[row + i][col] = 'S';
    }
}

void place_computer_ships() {
    for (int i = 0; i < NUM_SHIPS; i++) {
        int row, col;
        char orientation;
        do {
            row = rand() % BOARD_SIZE;
            col = rand() % BOARD_SIZE;
            orientation = (rand() % 2 == 0) ? 'H' : 'V';
        } while (!is_valid_placement(computer_board, row, col, ship_lengths[i], orientation));
        place_ship(computer_board, row, col, ship_lengths[i], orientation);
    }
}

void place_player_ships() {
    int row, col;
    char col_char, orientation;
    for (int i = 0; i < NUM_SHIPS; i++) {
        print_board(player_board);
        printf("Posicione seu %s (tamanho %d).\n", ship_names[i], ship_lengths[i]);
        do {
            printf("Digite a coordenada inicial (ex: A5): ");
            if(scanf(" %c%d", &col_char, &row)!=2){ while(getchar()!='\n');};
            col = toupper(col_char) - 'A';
            printf("Digite a orientacao (H-Horizontal, V-Vertical): ");
            scanf(" %c", &orientation);
            if (!is_valid(col, row) || !is_valid_placement(player_board, col, row, ship_lengths[i], orientation)) {
                printf("Posicao invalida. Tente novamente.\n");
            }
        } while (!is_valid(col, row) || !is_valid_placement(player_board, col, row, ship_lengths[i], orientation));
        place_ship(player_board, col, row, ship_lengths[i], orientation);
    }
}

void computer_turn() {
    int r, c;
    int shot_fired = 0;

    while(!shot_fired){
        if(ai_mode == HUNT){
            r = rand() % BOARD_SIZE;
            c = rand() % BOARD_SIZE;
            if (player_board[r][c] != '~' && player_board[r][c] != 'S') continue;
        } else { // TARGET MODE
            int dr[] = {-1, 1, 0, 0};
            int dc[] = {0, 0, -1, 1};
            int dir = -1;

            if(last_hit_row != first_hit_row || last_hit_col != first_hit_col){ // More than one hit
                int row_diff = last_hit_row - first_hit_row;
                int col_diff = last_hit_col - first_hit_col;
                r = last_hit_row + (row_diff != 0 ? (row_diff > 0 ? 1 : -1) : 0);
                c = last_hit_col + (col_diff != 0 ? (col_diff > 0 ? 1 : -1) : 0);

                if(!is_valid(r,c) || (player_board[r][c] != '~' && player_board[r][c] != 'S')){
                    r = first_hit_row - (row_diff != 0 ? (row_diff > 0 ? 1 : -1) : 0);
                    c = first_hit_col - (col_diff != 0 ? (col_diff > 0 ? 1 : -1) : 0);
                }
            } else { // First hit, try adjacent
                int random_dir_order[4] = {0,1,2,3};
                for(int i=0; i<4; i++){ int j = rand()%4; int temp=random_dir_order[i]; random_dir_order[i]=random_dir_order[j]; random_dir_order[j]=temp;}

                for(int i=0; i<4; i++){
                    dir = random_dir_order[i];
                    if(tried_dirs[dir] == 0) {
                        r = first_hit_row + dr[dir];
                        c = first_hit_col + dc[dir];
                        if (is_valid(r,c) && (player_board[r][c] == '~' || player_board[r][c] == 'S')) {
                            tried_dirs[dir] = 1;
                            break;
                        }
                    }
                    dir = -1;
                }
            }
            if(!is_valid(r, c) || (player_board[r][c] != '~' && player_board[r][c] != 'S')){
                 ai_mode = HUNT; // Could not find a valid next shot
                 for(int i=0; i<4; i++) tried_dirs[i]=0;
                 continue;
            }
        }

        printf("Computador ataca %c%d.\n", 'A' + r, c);
        shot_fired = 1;

        if (player_board[r][c] == 'S') {
            printf(">>> O COMPUTADOR ACERTOU SEU NAVIO! <<<\n");
            player_board[r][c] = 'X';
            if(ai_mode == HUNT) {
                ai_mode = TARGET;
                first_hit_row = r;
                first_hit_col = c;
            }
            last_hit_row = r;
            last_hit_col = c;
        } else {
            printf(">>> O COMPUTADOR ERROU! <<<\n");
            player_board[r][c] = 'O';
            if(ai_mode == TARGET){
                last_hit_row = first_hit_row;
                last_hit_col = first_hit_col;
            }
        }
    }
}

int check_win(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 'S') return 0;
        }
    }
    return 1;
}

int main() {
    srand(time(NULL));
    initialize_board(player_board);
    initialize_board(computer_board);
    initialize_board(player_view);
    
    place_computer_ships();
    place_player_ships();

    while (1) {
        printf("\nSEU TABULEIRO:\n");
        print_board(player_board);
        printf("TABULEIRO DO COMPUTADOR:\n");
        print_board(player_view);

        int row, col;
        char col_char;
        printf("Sua vez de atacar. Digite a coordenada (ex: A5): ");
        if(scanf(" %c%d", &col_char, &row) != 2){
            while(getchar()!='\n');
            printf("Entrada invalida. Tente novamente.\n");
            continue;
        }
        col = toupper(col_char) - 'A';

        if (!is_valid(col, row)) {
            printf("Coordenada invalida. Tente novamente.\n");
            continue;
        }

        if (player_view[col][row] != '~') {
            printf("Voce ja atacou esta posicao. Tente novamente.\n");
            continue;
        }

        if (computer_board[col][row] == 'S') {
            printf(">>> ACERTOU! <<<\n");
            player_view[col][row] = 'X';
            computer_board[col][row] = 'X';
        } else {
            printf(">>> AGUA! <<<\n");
            player_view[col][row] = 'O';
        }

        if (check_win(computer_board)) {
            printf("\nPARABENS! VOCE VENCEU A BATALHA!\n");
            print_board(player_view);
            break;
        }

        computer_turn();

        if (check_win(player_board)) {
            printf("\nVOCE PERDEU! O COMPUTADOR AFUNDOU TODOS OS SEUS NAVIOS.\n");
            print_board(player_board);
            break;
        }
    }

    return 0;
} 