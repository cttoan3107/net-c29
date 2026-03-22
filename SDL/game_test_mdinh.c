#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"
#include "queue.h"

int test_dummy() { return EXIT_SUCCESS; }

int test_game_print()
{
  game g = game_default();
  if (g == NULL) {
    fprintf(stderr, "Erreur : game_default() retourne NULL.\n");
    return EXIT_FAILURE;
  }
  game_print(g);
  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_default()
{
  game g = game_new_empty();
  assert(g);
  // create the corner shapes
  game_set_piece_shape(g, 0, 0, CORNER);
  game_set_piece_shape(g, 0, 3, CORNER);
  game_set_piece_shape(g, 3, 3, CORNER);
  // create the ENDPOINT shapes
  game_set_piece_shape(g, 0, 1, ENDPOINT);
  game_set_piece_shape(g, 0, 4, ENDPOINT);
  game_set_piece_shape(g, 0, 2, ENDPOINT);
  game_set_piece_shape(g, 2, 0, ENDPOINT);
  game_set_piece_shape(g, 2, 1, ENDPOINT);
  game_set_piece_shape(g, 2, 3, ENDPOINT);
  game_set_piece_shape(g, 3, 0, ENDPOINT);
  game_set_piece_shape(g, 4, 0, ENDPOINT);
  game_set_piece_shape(g, 4, 2, ENDPOINT);
  game_set_piece_shape(g, 4, 3, ENDPOINT);
  game_set_piece_shape(g, 4, 4, ENDPOINT);
  // create the SEGMENT shapes
  game_set_piece_shape(g, 2, 4, SEGMENT);
  game_set_piece_shape(g, 3, 4, SEGMENT);
  // create the TEE shapes
  game_set_piece_shape(g, 1, 0, TEE);
  game_set_piece_shape(g, 1, 1, TEE);
  game_set_piece_shape(g, 1, 2, TEE);
  game_set_piece_shape(g, 1, 3, TEE);
  game_set_piece_shape(g, 1, 4, TEE);
  game_set_piece_shape(g, 2, 2, TEE);
  game_set_piece_shape(g, 3, 1, TEE);
  game_set_piece_shape(g, 3, 2, TEE);
  game_set_piece_shape(g, 4, 1, TEE);
  // rotate the different pieces
  game_set_piece_orientation(g, 0, 0, WEST);
  game_set_piece_orientation(g, 0, 2, WEST);
  game_set_piece_orientation(g, 0, 4, SOUTH);
  game_set_piece_orientation(g, 1, 0, SOUTH);
  game_set_piece_orientation(g, 1, 1, WEST);
  game_set_piece_orientation(g, 1, 3, EAST);
  game_set_piece_orientation(g, 1, 4, EAST);
  game_set_piece_orientation(g, 2, 0, EAST);
  game_set_piece_orientation(g, 2, 2, WEST);
  game_set_piece_orientation(g, 2, 3, WEST);
  game_set_piece_orientation(g, 2, 4, EAST);
  game_set_piece_orientation(g, 3, 0, SOUTH);
  game_set_piece_orientation(g, 3, 1, SOUTH);
  game_set_piece_orientation(g, 3, 3, WEST);
  game_set_piece_orientation(g, 4, 0, EAST);
  game_set_piece_orientation(g, 4, 1, WEST);
  game_set_piece_orientation(g, 4, 2, SOUTH);
  game_set_piece_orientation(g, 4, 3, EAST);
  game_set_piece_orientation(g, 4, 4, SOUTH);
  game_print((cgame)g);
  game default_game = game_default();
  assert(default_game);
  int result = game_equal((cgame)default_game, (cgame)g, true);
  game_delete(g);
  game_delete(default_game);
  if (result) {
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

int test_game_default_solution()
{
  game g = game_default_solution();
  if (g == NULL) {
    fprintf(stderr, "Erreur : game_default_solution() retourne NULL.\n");
    return EXIT_FAILURE;
  }
  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_has_half_edge()
{
  game g = game_new_empty_ext(3, 3, false);

  // Test ENDPOINT pointing NORTH
  game_set_piece_shape(g, 0, 0, ENDPOINT);
  game_set_piece_orientation(g, 0, 0, NORTH);

  if (!game_has_half_edge(g, 0, 0, NORTH)) {
    fprintf(stderr, "Error: ENDPOINT (0, 0) should have a half edge to the NORTH.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  if (game_has_half_edge(g, 0, 0, EAST)) {
    fprintf(stderr, "Error: ENDPOINT (0, 0) should not have a half edge to the EAST.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  // Test SEGMENT oriented NORTH-SOUTH
  game_set_piece_shape(g, 1, 1, SEGMENT);
  game_set_piece_orientation(g, 1, 1, NORTH);

  if (!game_has_half_edge(g, 1, 1, NORTH) || !game_has_half_edge(g, 1, 1, SOUTH)) {
    fprintf(stderr, "Error: SEGMENT (1, 1) should have half edges to the NORTH and SOUTH.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  if (game_has_half_edge(g, 1, 1, EAST)) {
    fprintf(stderr, "Error: SEGMENT (1, 1) should not have a half edge to the EAST.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  // Test CROSS (should always have half edges in all directions)
  game_set_piece_shape(g, 2, 2, CROSS);

  if (!game_has_half_edge(g, 2, 2, NORTH) || !game_has_half_edge(g, 2, 2, EAST) || !game_has_half_edge(g, 2, 2, SOUTH) || !game_has_half_edge(g, 2, 2, WEST)) {
    fprintf(stderr, "Error: CROSS (2, 2) should have half edges in all directions.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);

  return EXIT_SUCCESS;
}
int test_game_check_edge()
{
  // Tạo một trò chơi 3x3 đơn giản
  game g = game_new_empty_ext(3, 3, false);

  // TH1: Không có cạnh nào
  if (game_check_edge(g, 1, 1, NORTH) != NOEDGE) return EXIT_FAILURE;

  // Đặt một nửa cạnh ở ô (1,1) hướng bắc
  game_set_piece_shape(g, 1, 1, ENDPOINT);
  game_set_piece_orientation(g, 1, 1, NORTH);

  // TH2: Một nửa cạnh -> MISMATCH
  if (game_check_edge(g, 1, 1, NORTH) != MISMATCH) return EXIT_FAILURE;

  // Đặt một nửa cạnh tương ứng ở ô phía trên (0,1)
  game_set_piece_shape(g, 0, 1, ENDPOINT);
  game_set_piece_orientation(g, 0, 1, SOUTH);

  // TH3: Hai nửa cạnh khớp -> MATCH
  if (game_check_edge(g, 1, 1, NORTH) != MATCH) return EXIT_FAILURE;

  // Đặt một nửa cạnh không khớp ở ô phía trên (0,1)
  game_set_piece_shape(g, 0, 1, ENDPOINT);
  game_set_piece_orientation(g, 0, 1, EAST);

  // TH4: Hai nửa cạnh không khớp -> MISMATCH
  if (game_check_edge(g, 1, 1, NORTH) != MISMATCH) return EXIT_FAILURE;

  // Giải phóng bộ nhớ
  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_get_ajacent_square()
{
  game g = game_new_empty_ext(4, 4, true);  // Jeu avec wrapping activé
  uint pi_next, pj_next;

  // Vérification avec wrapping
  if (!game_get_ajacent_square(g, 0, 0, NORTH, &pi_next, &pj_next) || pi_next != 3 || pj_next != 0) {
    fprintf(stderr, "Error: Wrapping NORTH failed.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  if (!game_get_ajacent_square(g, 3, 3, EAST, &pi_next, &pj_next) || pi_next != 3 || pj_next != 0) {
    fprintf(stderr, "Error: Wrapping EAST failed.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);

  // Cas sans wrapping
  g = game_new_empty_ext(4, 4, false);
  if (game_get_ajacent_square(g, 0, 0, NORTH, &pi_next, &pj_next)) {
    fprintf(stderr, "Error: NORTH should not exist without wrapping.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_is_well_paired()
{
  game g = game_default();
  bool res = game_is_well_paired(g);

  if (res == true) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  game_delete(g);
  g = game_default_solution();

  if (g == false) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);

  return EXIT_SUCCESS;
}
int test_game_new_ext()
{
  uint rows = 4, cols = 3;
  shape shapes[12] = {CORNER, TEE, SEGMENT, ENDPOINT, EMPTY, CROSS, SEGMENT, CORNER, TEE, CROSS, ENDPOINT, EMPTY};
  direction orientations[12] = {NORTH, EAST, SOUTH, WEST, NORTH, EAST, SOUTH, WEST, NORTH, EAST, SOUTH, WEST};

  game g = game_new_ext(rows, cols, shapes, orientations, true);
  if (!g) {
    fprintf(stderr, "Error: game_new_ext returned NULL.\n");
    return EXIT_FAILURE;
  }

  if (game_nb_rows(g) != rows || game_nb_cols(g) != cols) {
    fprintf(stderr, "Error: Dimensions mismatch in game_new_ext.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  if (!game_is_wrapping(g)) {
    fprintf(stderr, "Error: Wrapping should be enabled in game_new_ext.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      uint index = i * cols + j;
      if (game_get_piece_shape(g, i, j) != shapes[index] || game_get_piece_orientation(g, i, j) != orientations[index]) {
        fprintf(stderr, "Error: Incorrect shape or orientation at (%u, %u).\n", i, j);
        game_delete(g);
        return EXIT_FAILURE;
      }
    }
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_new_empty_ext()
{
  uint rows = 5, cols = 5;
  game g = game_new_empty_ext(rows, cols, false);

  if (!g) {
    fprintf(stderr, "Error: game_new_empty_ext returned NULL.\n");
    return EXIT_FAILURE;
  }

  if (game_nb_rows(g) != rows || game_nb_cols(g) != cols) {
    fprintf(stderr, "Error: Dimensions mismatch in game_new_empty_ext.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  if (game_is_wrapping(g)) {
    fprintf(stderr, "Error: Wrapping should be disabled in game_new_empty_ext.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      if (game_get_piece_shape(g, i, j) != EMPTY || game_get_piece_orientation(g, i, j) != NORTH) {
        fprintf(stderr, "Error: Incorrect default shape or orientation at (%u, %u).\n", i, j);
        game_delete(g);
        return EXIT_FAILURE;
      }
    }
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_is_wrapping()
{
  game g = game_new_empty_ext(3, 3, true);  // Wrapping activé
  if (!g) return EXIT_FAILURE;

  if (!game_is_wrapping(g)) {
    fprintf(stderr, "Error: Wrapping expected to be enabled.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_undo()
{
  game g = game_new_empty_ext(3, 3, false);
  if (!g) {
    return EXIT_FAILURE;
  }

  game_play_move(g, 0, 0, 1);
  if (game_get_piece_orientation(g, 0, 0) != EAST) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_undo(g);
  if (game_get_piece_orientation(g, 0, 0) != NORTH) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_redo()
{
  game g = game_new_empty_ext(3, 3, false);
  if (!g) {
    return EXIT_FAILURE;
  }

  game_play_move(g, 0, 0, 1);
  if (game_get_piece_orientation(g, 0, 0) != EAST) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_undo(g);
  if (game_get_piece_orientation(g, 0, 0) != NORTH) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  game_redo(g);
  if (game_get_piece_orientation(g, 0, 0) != EAST) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_load(void)
{
  game g3 = game_default();
  game g4 = game_default_solution();

  game_save(g3, "default.txt");
  game_save(g4, "default_sol.txt");

  game g1 = game_load("default.txt");
  game g2 = game_load("default_sol.txt");

  if (!g1 || !g2) {
    fprintf(stderr, "Failed to load games.\n");
    return EXIT_FAILURE;
  }

  bool test1 = game_equal(g1, g3, false);
  bool test2 = game_equal(g2, g4, false);
  bool test3 = game_won(g2);

  game_delete(g1);
  game_delete(g2);
  game_delete(g3);
  game_delete(g4);

  return (test1 && test2 && test3) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int test_game_save()
{
  game g = game_default();
  game_play_move(g, 4, 4, 2);
  game_save(g, "test_file");

  game g1 = game_default();
  game g2 = game_load("test_file");

  if (g2 == NULL) {
    fprintf(stderr, "Error: game_load failed.\n");
    game_delete(g);
    game_delete(g1);
    return EXIT_FAILURE;
  }

  bool test1 = !game_equal(g1, g2, false);
  game_play_move(g1, 4, 4, 2);
  bool test2 = game_equal(g1, g2, false);

  game_delete(g);
  game_delete(g1);
  game_delete(g2);

  return (test1 && test2) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <testname>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "dummy") == 0) {
    return test_dummy();
  } else if (strcmp(argv[1], "game_print") == 0) {
    return test_game_print();
  } else if (strcmp(argv[1], "game_default") == 0) {
    return test_game_default();
  } else if (strcmp(argv[1], "game_default_solution") == 0) {
    return test_game_default_solution();
  } else if (strcmp(argv[1], "game_get_ajacent_square") == 0) {
    return test_game_get_ajacent_square();
  } else if (strcmp(argv[1], "game_has_half_edge") == 0) {
    return test_game_has_half_edge();
  } else if (strcmp(argv[1], "game_check_edge") == 0) {
    return test_game_check_edge();
  } else if (strcmp(argv[1], "game_is_well_paired") == 0) {
    return test_game_is_well_paired();
  } else if (strcmp(argv[1], "game_new_ext") == 0) {
    return test_game_new_ext();
  } else if (strcmp(argv[1], "game_new_empty_ext") == 0) {
    return test_game_new_empty_ext();
  } else if (strcmp(argv[1], "game_is_wrapping") == 0) {
    return test_game_is_wrapping();
  } else if (strcmp(argv[1], "game_undo") == 0) {
    return test_game_undo();
  } else if (strcmp(argv[1], "game_redo") == 0) {
    return test_game_redo();
  } else if (strcmp(argv[1], "game_load") == 0) {
    return test_game_load();
  } else if (strcmp(argv[1], "game_save") == 0) {
    return test_game_save();
  } else {
    fprintf(stderr, "Unknown test '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
}
