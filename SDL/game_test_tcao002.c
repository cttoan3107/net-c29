#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "queue.h"

int test_dummy() { return EXIT_SUCCESS; }
int test_game_new_empty()
{
  game g = game_new_empty();
  if (g == NULL) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  for (uint i = 0; i < DEFAULT_SIZE; i++) {
    for (uint j = 0; j < DEFAULT_SIZE; j++) {
      if (game_get_piece_shape(g, i, j) != EMPTY || game_get_piece_orientation(g, i, j) != NORTH) {
        game_delete(g);
        return EXIT_FAILURE;
      }
    }
  }
  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_new()
{
  shape shapes[DEFAULT_SIZE * DEFAULT_SIZE] = {SEGMENT, CORNER,   TEE,     EMPTY,  ENDPOINT, SEGMENT, CORNER,   TEE,     EMPTY,  ENDPOINT, SEGMENT, CORNER,  TEE,
                                               EMPTY,   ENDPOINT, SEGMENT, CORNER, TEE,      EMPTY,   ENDPOINT, SEGMENT, CORNER, TEE,      EMPTY,   ENDPOINT};

  direction orientations[DEFAULT_SIZE * DEFAULT_SIZE] = {NORTH, EAST,  SOUTH, WEST,  NORTH, EAST,  SOUTH, WEST,  NORTH, EAST,  SOUTH, WEST, NORTH,
                                                         EAST,  SOUTH, WEST,  NORTH, EAST,  SOUTH, WEST,  NORTH, EAST,  SOUTH, WEST,  NORTH};

  game g = game_new(shapes, orientations);
  if (!g) {
    return EXIT_FAILURE;
  }
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_piece_shape(g, i, j) != shapes[i * g->nb_cols + j] || game_get_piece_orientation(g, i, j) != orientations[i * g->nb_cols + j]) {
        game_delete(g);
        return EXIT_FAILURE;
      }
    }
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_copy()
{
  // Create a game with wrapping and specific shapes/orientations
  uint rows = 4, cols = 4;
  shape shapes[16] = {SEGMENT, CORNER, TEE, EMPTY, ENDPOINT, CROSS, SEGMENT, CORNER, TEE, ENDPOINT, SEGMENT, TEE, EMPTY, ENDPOINT, CORNER, SEGMENT};
  direction orientations[16] = {NORTH, EAST, SOUTH, WEST, EAST, SOUTH, NORTH, WEST, NORTH, EAST, SOUTH, WEST, EAST, NORTH, WEST, SOUTH};

  game original_game = game_new_ext(rows, cols, shapes, orientations, true);
  if (!original_game) {
    fprintf(stderr, "Error: Failed to create original game.\n");
    return EXIT_FAILURE;
  }

  // Copy the game
  game copied_game = game_copy(original_game);
  if (!copied_game) {
    fprintf(stderr, "Error: Failed to copy game.\n");
    game_delete(original_game);
    return EXIT_FAILURE;
  }

  // Verify dimensions
  if (game_nb_rows(original_game) != game_nb_rows(copied_game) || game_nb_cols(original_game) != game_nb_cols(copied_game)) {
    fprintf(stderr, "Error: Dimensions mismatch in copied game.\n");
    game_delete(original_game);
    game_delete(copied_game);
    return EXIT_FAILURE;
  }

  // Verify wrapping
  if (game_is_wrapping(original_game) != game_is_wrapping(copied_game)) {
    fprintf(stderr, "Error: Wrapping mismatch in copied game.\n");
    game_delete(original_game);
    game_delete(copied_game);
    return EXIT_FAILURE;
  }

  // Verify shapes and orientations
  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      if (game_get_piece_shape(original_game, i, j) != game_get_piece_shape(copied_game, i, j) || game_get_piece_orientation(original_game, i, j) != game_get_piece_orientation(copied_game, i, j)) {
        fprintf(stderr, "Error: Mismatch in shapes or orientations at (%u, %u).\n", i, j);
        game_delete(original_game);
        game_delete(copied_game);
        return EXIT_FAILURE;
      }
    }
  }

  // Modify the original game and ensure the copy remains unchanged
  game_set_piece_shape(original_game, 0, 0, EMPTY);
  if (game_get_piece_shape(original_game, 0, 0) == game_get_piece_shape(copied_game, 0, 0)) {
    fprintf(stderr, "Error: Copied game is not independent of the original.\n");
    game_delete(original_game);
    game_delete(copied_game);
    return EXIT_FAILURE;
  }

  game_delete(original_game);
  game_delete(copied_game);
  return EXIT_SUCCESS;
}

int test_game_set_piece_shape()
{
  game g = game_new_empty();
  game_set_piece_shape(g, 1, 1, CORNER);
  if (game_get_piece_shape(g, 1, 1) != CORNER) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_won()
{
  printf("Running test_game_won...\n");

  game g = game_default();
  if (game_won(g)) {
    fprintf(stderr, "Error: Default game should not be won.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);

  g = game_default_solution();
  if (!game_won(g)) {
    fprintf(stderr, "Error: Default solution should be won.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  printf("All test_game_won cases passed!\n");
  return EXIT_SUCCESS;
}

int test_game_reset_orientation()
{
  uint test_width = 5;
  uint test_height = 5;
  shape* shapes = malloc(test_width * test_height * sizeof(shape));
  direction* orientations = malloc(test_width * test_height * sizeof(direction));

  for (uint i = 0; i < test_width * test_height; i++) {
    shapes[i] = (i % 5 == 0) ? SEGMENT : (i % 5 == 1) ? CORNER : (i % 5 == 2) ? TEE : (i % 5 == 3) ? EMPTY : ENDPOINT;
    orientations[i] = (i % 4 == 0) ? NORTH : (i % 4 == 1) ? EAST : (i % 4 == 2) ? SOUTH : WEST;
  }

  game g = game_new(shapes, orientations);
  free(shapes);
  free(orientations);

  game_reset_orientation(g);
  for (uint i = 0; i < test_height; i++) {
    for (uint j = 0; j < test_width; j++) {
      if (game_get_piece_orientation(g, i, j) != NORTH) {
        game_delete(g);
        return EXIT_FAILURE;
      }
    }
  }
  game_delete(g);
  return EXIT_SUCCESS;
}
#define NB_TESTS 100
int test_game_shuffle_orientation()
{
  // Création d'un jeu de test
  uint nb_rows = 3, nb_cols = 3;
  shape shapes[] = {CROSS, SEGMENT, ENDPOINT, CORNER, EMPTY, TEE, SEGMENT, ENDPOINT, CROSS};
  direction dirs[] = {NORTH, EAST, SOUTH, WEST, NORTH, EAST, SOUTH, WEST, NORTH};

  game g = game_new_ext(nb_rows, nb_cols, shapes, dirs, false);
  if (!g) {
    fprintf(stderr, "Erreur lors de la création du jeu.\n");
    return EXIT_FAILURE;
  }

  // Sauvegarde des directions initiales
  direction original_dirs[nb_rows * nb_cols];
  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      original_dirs[i * nb_cols + j] = game_get_piece_orientation(g, i, j);
    }
  }

  // Appliquer le mélange plusieurs fois
  for (int test = 0; test < NB_TESTS; test++) {
    game_shuffle_orientation(g);

    // Vérifier que les cases vides n'ont pas changé
    for (uint i = 0; i < nb_rows; i++) {
      for (uint j = 0; j < nb_cols; j++) {
        if (game_get_piece_shape(g, i, j) == EMPTY) {
          if (game_get_piece_orientation(g, i, j) != original_dirs[i * nb_cols + j]) {
            fprintf(stderr, "Erreur: la direction d'une case vide a changé.\n");
            game_delete(g);
            return EXIT_FAILURE;
          }
        }
      }
    }
  }

  game_delete(g);
  printf("Test game_shuffle_orientation passé avec succès !\n");
  return EXIT_SUCCESS;
}

int test_game_nb_rows()
{
  uint nb_rows = 5;
  uint nb_cols = 4;
  game g = game_new_empty_ext(nb_rows, nb_cols, false);
  if (g == NULL) {
    return EXIT_FAILURE;
  }

  if (game_nb_rows(g) != nb_rows) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_nb_cols()
{
  uint nb_rows = 5;
  uint nb_cols = 4;
  game g = game_new_empty_ext(nb_rows, nb_cols, false);
  if (g == NULL) {
    return EXIT_FAILURE;
  }

  if (game_nb_cols(g) != nb_cols) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    printf("Usage: %s <test_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "dummy") == 0) {
    return test_dummy();
  } else if (strcmp(argv[1], "game_new_empty") == 0) {
    return test_game_new_empty();
  } else if (strcmp(argv[1], "game_new") == 0) {
    return test_game_new();
  } else if (strcmp(argv[1], "game_copy") == 0) {
    return test_game_copy();
  } else if (strcmp(argv[1], "game_set_piece_shape") == 0) {
    return test_game_set_piece_shape();
  } else if (strcmp(argv[1], "game_won") == 0) {
    return test_game_won();
  } else if (strcmp(argv[1], "game_reset_orientation") == 0) {
    return test_game_reset_orientation();
  } else if (strcmp(argv[1], "game_shuffle_orientation") == 0) {
    return test_game_shuffle_orientation();
  } else if (strcmp(argv[1], "game_nb_cols") == 0) {
    return test_game_nb_cols();
  } else if (strcmp(argv[1], "game_nb_rows") == 0) {
    return test_game_nb_rows();
  } else {
    printf("Invalid test name: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
}