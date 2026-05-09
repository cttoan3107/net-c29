#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_tools.h"

void usage(int argc, char* argv[])
{
  fprintf(stderr, "Usage: %s <option> <input> [<output>]\n", argv[0]);
  fprintf(stderr, "Where: <input> is a game file, [<output>] is the name of resulting file that will be created\n");
  fprintf(stderr, "Example: %s -s my_game.txt solved.sol\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
  if (argc < 3 || argc > 4) usage(argc, argv);

  if (strcmp(argv[1], "-s") == 0) {
    game g = game_load(argv[2]);
    if (!g) {
      fprintf(stderr, "Error: Failed to load the game from file %s\n", argv[2]);
      return EXIT_FAILURE;
    }

    bool game_is_solved = game_solve(g);
    if (!game_is_solved) {
      printf("No solution found!\n");
      game_delete(g);
      return EXIT_FAILURE;
    }

    game_save(g, argv[3]);

  } else {
    game g = game_load(argv[2]);
    if (!g) {
      fprintf(stderr, "Error: Failed to load the game from file %s\n", argv[2]);
      return EXIT_FAILURE;
    }

    int nb_sol = game_nb_solutions(g);
    FILE* file = fopen(argv[3], "w");
    if (file == NULL) {
      fprintf(stderr, "Error: Cannot open output file %s\n", argv[3]);
      game_delete(g);
      return EXIT_FAILURE;
    }

    fprintf(file, "%d\n", nb_sol);
    fclose(file);
    printf("Number of solutions saved to %s.\n", argv[3]);

    game_delete(g);
  }

  return EXIT_SUCCESS;
}
