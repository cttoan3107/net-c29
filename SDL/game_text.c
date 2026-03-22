#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"
#include "queue.h"

/**
 * @brief Affiche les instructions pour les joueurs.
 */
void print_help()
{
  printf("> action: help\n");
  printf(" - press 'c <i> <j>' to rotate piece clockwise in square (i, j)\n");
  printf(" - press 'a <i> <j>' to rotate piece anti-clockwise in square (i, j)\n");
  printf(" - press 'r' to shuffle game\n");
  printf(" - press 'z' to undo last move\n");
  printf(" - press 'y' to redo last undone move\n");
  printf(" - press 's <filename>' to save the game state to a file\n");
  printf(" - press 'q' to quit the game\n");
}

/**
 * @brief Fonction principale du jeu.
 * @return Code de retour du programme.
 */
int main(int argc, char* argv[])
{
  game g;

  // Vérifier si un fichier est passé en argument
  if (argc > 1) {
    // Charger le jeu depuis le fichier spécifié
    g = game_load(argv[1]);
    if (g == NULL) {
      fprintf(stderr, "Erreur: Impossible de charger le jeu à partir du fichier '%s'.\n", argv[1]);
      return EXIT_FAILURE;
    }
  } else {
    // Charger le jeu par défaut
    g = game_default();
    if (g == NULL) {
      fprintf(stderr, "Erreur: Impossible d'initialiser le jeu.\n");
      return EXIT_FAILURE;
    }
  }

  char command;
  int i, j;
  char filename[256];  // Buffer pour le nom du fichier

  // Boucle principale du jeu
  while (!game_won(g)) {
    // Afficher la grille actuelle
    game_print(g);

    printf("> ? [h for help]:\n");
    if (scanf(" %c", &command) != 1) {
      fprintf(stderr, "Error reading input.\n");
      game_delete(g);
      return EXIT_FAILURE;
    }

    if (command == 'h') {
      print_help();  // Afficher l'aide
    } else if (command == 'r') {
      printf("> action: restart\n");
      game_shuffle_orientation(g);  // Réinitialiser la grille
    } else if (command == 'q') {
      printf("> action: quit\n");
      printf("shame! You gave up\n");
      game_delete(g);
      return EXIT_SUCCESS;
    } else if (command == 'c' || command == 'a') {
      if (scanf("%d %d", &i, &j) == 2) {
        // Vérifier que les coordonnées sont valides
        if (i >= 0 && i < game_nb_rows(g) && j >= 0 && j < game_nb_cols(g)) {
          if (command == 'c') {
            game_play_move(g, i, j, 1);  // Tourner dans le sens horaire
            printf("> action: play move 'c' into square (%d, %d)\n", i, j);
          } else {
            game_play_move(g, i, j, -1);  // Tourner dans le sens antihoraire
            printf("> action: play move 'a' into square (%d, %d)\n", i, j);
          }
        } else {
          printf("Erreur: Coordonnées hors limites (0-9).\n");
        }
      } else {
        fprintf(stderr, "Error: Invalid coordinates.\n");
        game_delete(g);
        return EXIT_FAILURE;
      }

    } else if (command == 'z') {
      game_undo(g);  // Annuler le dernier mouvement
      printf("> action: Undo\n");
    } else if (command == 'y') {
      game_redo(g);  // Refaire le dernier mouvement annulé
      printf("> action: Redo\n");
    } else if (command == 's') {
      // Commande pour sauvegarder l'état du jeu dans un fichier
      if (scanf("%s", filename) == 1) {
        game_save(g, filename);  // Sauvegarder le jeu dans le fichier spécifié
        printf("> action: saved game state to '%s'\n", filename);
      } else {
        fprintf(stderr, "Error: Invalid filename.\n");
        game_delete(g);
        return EXIT_FAILURE;
      }
    } else {
      printf("Invalid command. Type 'h' for help.\n");
    }
  }

  // Afficher la grille finale lorsque le joueur gagne
  game_print(g);
  printf("Congratulation! You are winner\n");
  game_delete(g);
  return EXIT_SUCCESS;
}
