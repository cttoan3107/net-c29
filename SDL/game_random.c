#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_tools.h"

int main(int argc, char* argv[])
{
  // Vérifier le nombre d'arguments
  if (argc < 7) {
    printf("Usage: ./game_random <nb_rows> <nb_cols> <wrapping> <nb_empty> <nb_extra> <shuffle> [<filename>]\n");
    printf("Example: ./game_random 4 4 0 0 0 0 random.sol\n");
    return 1;
  }
  // Initialisation du générateur aléatoire
  srand(time(NULL));

  // Extraire les arguments
  uint nb_rows = strtoul(argv[1], NULL, 10);
  uint nb_cols = strtoul(argv[2], NULL, 10);
  bool wrapping = strtoul(argv[3], NULL, 10);
  uint nb_empty = strtoul(argv[4], NULL, 10);
  uint nb_extra = strtoul(argv[5], NULL, 10);
  bool shuffle = strtoul(argv[6], NULL, 10);
  char* filename = (argc > 7) ? argv[7] : NULL;

  // Affichage des paramètres
  printf("Copyright: Net Game by University of Bordeaux, 2024.\n");
  printf("nb_rows=%u nb_cols=%u wrapping=%u\n", nb_rows, nb_cols, wrapping);
  printf("nb_empty=%u nb_extra=%u shuffle=%u\n", nb_empty, nb_extra, shuffle);

  // Création du jeu
  game g = game_random(nb_rows, nb_cols, wrapping, nb_empty, nb_extra);
  if (!g) {
    printf("Error generating game!\n");
    return 1;
  }

  // Mélanger les orientations si demandé
  if (shuffle) {
    game_shuffle_orientation(g);
  }

  // Affichage du jeu
  game_print(g);

  // Sauvegarde du jeu dans un fichier si demandé
  if (filename) {
    game_save(g, filename);
    printf("Game saved to %s\n", filename);
  }

  // Libération de la mémoire
  game_delete(g);

  return 0;
}