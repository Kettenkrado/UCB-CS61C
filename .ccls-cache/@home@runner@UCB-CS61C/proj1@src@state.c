#include "state.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

#define MAX_CHAR 100010

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

static void create_snake(unsigned int tail_row, unsigned int tail_col,
                         unsigned int head_row, unsigned int head_col,
                         snake_t *new_snake, char **board, char direction);

/* Task 1 */
game_state_t *create_default_state() {
  game_state_t *default_state = malloc(sizeof(game_state_t));
  if (default_state == NULL) {
    printf("No memory available.");
    exit(1);
  }
  unsigned int num_cols = 20;

  default_state->num_rows = 18;
  default_state->board = malloc(18 * sizeof(char *));

  for (int i = 0; i < default_state->num_rows; i++) {
    *(default_state->board + i) = malloc((num_cols + 2) * sizeof(char));

    for (int j = 1; j < (num_cols - 1); j++)
      *(*(default_state->board + i) + j) = ' ';

    if (i == 0 || i == 17) {
      for (int j = 1; j < (num_cols - 1); j++)
        *(*(default_state->board + i) + j) = '#';
    } else if (i == 2) {
      *(*(default_state->board + i) + 9) = '*';
    }

    *(*(default_state->board + i) + 0) = '#';
    *(*(default_state->board + i) + (num_cols - 1)) = '#';
    *(*(default_state->board + i) + (num_cols)) = '\n';
    *(*(default_state->board + i) + (num_cols + 1)) = '\0';
  }

  default_state->num_snakes = 1;
  default_state->snakes = malloc(sizeof(snake_t) * default_state->num_snakes);
  create_snake(2, 2, 2, 4, default_state->snakes, default_state->board, 'd');

  return default_state;
}

static void create_snake(unsigned int tail_row, unsigned int tail_col,
                         unsigned int head_row, unsigned int head_col,
                         snake_t *new_snake, char **board, char direction) {
  new_snake->tail_row = tail_row;
  new_snake->tail_col = tail_col;
  new_snake->head_col = head_col;
  new_snake->head_row = head_row;
  new_snake->live = true;

  board[tail_row][tail_col] = direction;
  board[head_row][head_col] = (char)toupper(direction);

  switch (direction) {
  case 'a':
    board[head_row][head_col + 1] = '<';
    break;
  case 'd':
    board[head_row][head_col - 1] = '>';
    break;
  case 'w':
    board[head_row + 1][head_col] = '^';
    break;
  case 's':
    board[head_row - 1][head_col] = 'v';
  }
}

/* Task 2 */
void free_state(game_state_t *state) {
  free(state->snakes);
  for (int i = 0; i < state->num_rows; i++)
    free(*(state->board + i));
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s", *(state->board + i));
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x';
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return is_tail(c) || is_head(c) || c == '<' || c == '>' || c == '^' ||
         c == 'v';
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
  case '<':
    return 'a';
    break;
  case '>':
    return 'd';
    break;
  case 'v':
    return 's';
    break;
  case '^':
    return 'w';
    break;
  default:
    return ' ';
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
  case 'A':
    return '<';
    break;
  case 'D':
    return '>';
    break;
  case 'W':
    return '^';
    break;
  case 'S':
    return 'v';
    break;
  default:
    return ' ';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S')
    return cur_row + 1;
  else if (c == '^' || c == 'w' || c == 'W')
    return cur_row - 1;
  else
    return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D')
    return cur_col + 1;
  else if (c == '<' || c == 'a' || c == 'A')
    return cur_col - 1;
  else
    return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake
  is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  unsigned int next_row, next_col;
  char head;
  snake_t *snake = state->snakes + snum;

  head = state->board[snake->head_row][snake->head_col];
  next_row = get_next_row(snake->head_row, head);
  next_col = get_next_col(snake->head_col, head);

  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  snake_t *snake = state->snakes + snum;
  char head = state->board[snake->head_row][snake->head_col];
  unsigned int next_row = get_next_row(snake->head_row, head);
  unsigned int next_col = get_next_col(snake->head_col, head);

  state->board[next_row][next_col] = head;
  state->board[snake->head_row][snake->head_col] = head_to_body(head);
  snake->head_row = next_row;
  snake->head_col = next_col;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  snake_t *snake = state->snakes + snum;
  char tail = state->board[snake->tail_row][snake->tail_col], new_tail;
  unsigned int next_row = get_next_row(snake->tail_row, tail);
  unsigned int next_col = get_next_col(snake->tail_col, tail);

  new_tail = body_to_tail(state->board[next_row][next_col]);
  state->board[snake->tail_row][snake->tail_col] =
      ' '; // blank out the current tail
  state->board[next_row][next_col] =
      new_tail; // change the new tail on the board
  snake->tail_row = next_row;
  snake->tail_col = next_col;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    if (!state->snakes[i].live)
      continue;

    char next_char = next_square(state, i);
    if (next_char == ' ') {
      update_head(state, i);
      update_tail(state, i);
    } else if (next_char == '*') {
      update_head(state, i);
      add_food(state);
    } else {
      snake_t *snake = state->snakes + i;
      snake->live = false;
      state->board[snake->head_row][snake->head_col] = 'x';
    }
  }
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  unsigned int max_length = MAX_CHAR;
  char *new_line = malloc(max_length * sizeof(char));

  if (fgets(new_line, (int)max_length, fp)) {
    unsigned int exact_length = (unsigned int)strlen(new_line);
    new_line = realloc(new_line, exact_length + 1);
    return new_line;
  } else {
    free(new_line);
    return NULL;
  }
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  game_state_t *state = malloc(sizeof(game_state_t));
  unsigned int max_num_rows = MAX_CHAR;

  state->num_snakes = 0;
  state->snakes = NULL;
  state->num_rows = 0;
  state->board = malloc(max_num_rows * sizeof(char *));

  while (true) {
    char *line = read_line(fp);
    if (!line) {
      break;
    }

    *(state->board + state->num_rows) = line;
    state->num_rows++;
  }

  state->board = realloc(state->board, (state->num_rows + 1) * sizeof(char *));

  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  snake_t *snake = state->snakes + snum;
  char current_char = state->board[snake->tail_row][snake->tail_col];
  unsigned int current_row = snake->tail_row;
  unsigned int current_col = snake->tail_col;

  while (!is_head(current_char)) {
    current_row = get_next_row(current_row, current_char);
    current_col = get_next_col(current_col, current_char);
    current_char = state->board[current_row][current_col];
  }

  snake->head_row = current_row;
  snake->head_col = current_col;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  unsigned int max_num_snakes = MAX_CHAR;
  state->snakes = malloc(sizeof(snake_t) * max_num_snakes);

  for (unsigned int i = 0; i < state->num_rows; i++) {
    char *this_line = *(state->board + i);
    for (unsigned int j = 0; j < (unsigned int)strlen(this_line); j++) {
      if (is_tail(this_line[j])) {
        state->snakes[state->num_snakes].tail_row = i;
        state->snakes[state->num_snakes].tail_col = j;
        state->num_snakes++;
      }
    }
  }

  for (unsigned int i = 0; i < state->num_snakes; i++) {
    find_head(state, i);
    if (state->board[state->snakes[i].head_row][state->snakes[i].head_col] ==
        'x')
      state->snakes[i].live = false;
    else {
      state->snakes[i].live = true;
    }
  }

  state->snakes = realloc(state->snakes, state->num_snakes * sizeof(snake_t));
  return state;
}
