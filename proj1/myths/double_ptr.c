#include <stdio.h>
#include <stdlib.h>

int main() {
  char **board = malloc(18*sizeof(char *));
  //char *line5 = *(board + 5);
  //line5 = calloc(22, sizeof(char));

  *(board + 5) = calloc(22, sizeof(char));
  char *line5 = *(board + 5);

  *(*(board + 5)) = '#';
  *(line5 + 1) = 'm';

  printf("%s", line5);
  
  free(line5);
  free(board);
  return 0;
}
