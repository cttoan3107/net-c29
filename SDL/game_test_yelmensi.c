#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "queue.h"

int test_dummy(void) { return EXIT_SUCCESS; }

int test_game_is_connected()
{
  game g = game_new_empty_ext(4, 4, true);  // Wrapping enabled
  if (!g) return EXIT_FAILURE;

  // Create a simple connected structure with wrapping
  game_set_piece_shape(g, 0, 0, SEGMENT);
  game_set_piece_orientation(g, 0, 0, EAST);
  game_set_piece_shape(g, 0, 3, SEGMENT);
  game_set_piece_orientation(g, 0, 3, WEST);

  if (!game_is_connected(g)) {
    fprintf(stderr, "Error: Connected test failed with wrapping.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  // Disconnect the structure
  game_set_piece_shape(g, 0, 3, EMPTY);
  if (game_is_connected(g)) {
    fprintf(stderr, "Error: Disconnected test failed with wrapping.\n");
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_play_move()
{
  game g = game_new_empty();
  if (!g) {
    fprintf(stderr, "Error: Game initialization failed.\n");
    return EXIT_FAILURE;
  }

  game_set_piece_orientation(g, 0, 0, NORTH);
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

int test_game_get_piece_shape()
{
  game g = game_new_empty_ext(5, 5, false);

  game_set_piece_shape(g, 0, 0, CORNER);
  if (game_get_piece_shape(g, 0, 0) != CORNER) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_shape(g, 2, 2, TEE);
  if (game_get_piece_shape(g, 2, 2) != TEE) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_shape(g, 4, 4, ENDPOINT);
  if (game_get_piece_shape(g, 4, 4) != ENDPOINT) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_shape(g, 0, 1, SEGMENT);
  if (game_get_piece_shape(g, 0, 1) != SEGMENT) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_shape(g, 3, 3, EMPTY);
  if (game_get_piece_shape(g, 3, 3) != EMPTY) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_get_piece_orientation()
{
  game g = game_default();
  if (game_get_piece_orientation(g, 0, 0) != WEST) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  if (game_get_piece_orientation(g, 0, 4) != SOUTH) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  if (game_get_piece_orientation(g, 3, 3) != WEST) {
    game_delete(g);
    return EXIT_FAILURE;
  }
  if (game_get_piece_orientation(g, 4, 2) != SOUTH) {
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

int test_game_set_piece_orientation()
{
  game g = game_new_empty();
  if (!g) {
    fprintf(stderr, "Failed to create game instance.\n");
    exit(EXIT_FAILURE);
  }

  // Test case 1: Set and get valid orientation
  game_set_piece_orientation(g, 2, 3, EAST);
  if (game_get_piece_orientation(g, 2, 3) != EAST) {
    fprintf(stderr, "Test case 1 failed: Orientation mismatch.\n");
    game_delete(g);
    exit(EXIT_FAILURE);
  }

  // Test case 2: Set another valid orientation
  game_set_piece_orientation(g, 4, 1, SOUTH);
  if (game_get_piece_orientation(g, 4, 1) != SOUTH) {
    fprintf(stderr, "Test case 2 failed: Orientation mismatch.\n");
    game_delete(g);
    exit(EXIT_FAILURE);
  }

  // Test case 3: Setting NORTH to a piece and checking
  game_set_piece_orientation(g, 0, 0, NORTH);
  if (game_get_piece_orientation(g, 0, 0) != NORTH) {
    fprintf(stderr, "Test case 3 failed: Orientation mismatch.\n");
    game_delete(g);
    exit(EXIT_FAILURE);
  }

// Test case 4: Invalid indices (should handle gracefully, assuming the function checks bounds)
#ifdef CHECK_BOUNDS
  if (game_set_piece_orientation(g, DEFAULT_SIZE, DEFAULT_SIZE, WEST)) {
    fprintf(stderr, "Test case 4 failed: Out-of-bounds index not handled.\n");
    game_delete(g);
    exit(EXIT_FAILURE);
  }
#endif

  game_delete(g);
  printf("All tests passed!\n");
  exit(EXIT_SUCCESS);
}

int test_game_delete()
{
  game g = game_default();
  if (!g) {
    fprintf(stderr, "Error: game_default() returned NULL.\n");
    return EXIT_FAILURE;
  }

  game_delete(g);
  fprintf(stderr, "game_delete executed without crashing.\n");
  return EXIT_SUCCESS;
}

int test_game_equal()
{
  // Create two identical games
  game g1 = game_new_empty_ext(4, 4, false);
  game g2 = game_new_empty_ext(4, 4, false);

  // Set the same shapes and orientations for both games
  game_set_piece_shape(g1, 0, 0, CORNER);
  game_set_piece_orientation(g1, 0, 0, NORTH);
  game_set_piece_shape(g2, 0, 0, CORNER);
  game_set_piece_orientation(g2, 0, 0, NORTH);

  game_set_piece_shape(g1, 1, 1, TEE);
  game_set_piece_orientation(g1, 1, 1, EAST);
  game_set_piece_shape(g2, 1, 1, TEE);
  game_set_piece_orientation(g2, 1, 1, EAST);

  // Check if the two games are equal
  if (!game_equal((cgame)g1, (cgame)g2, false)) {
    game_delete(g1);
    game_delete(g2);
    return EXIT_FAILURE;
  }

  // Modify one game and ensure they are no longer equal
  game_set_piece_shape(g1, 2, 2, ENDPOINT);
  if (game_equal((cgame)g1, (cgame)g2, false)) {
    game_delete(g1);
    game_delete(g2);
    return EXIT_FAILURE;
  }

  // Cleanup
  game_delete(g1);
  game_delete(g2);
  return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
  int res;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <testname>\n", argv[0]);
    return (EXIT_FAILURE);
  }
  if (!strcmp(argv[1], "dummy")) res = test_dummy();
  /*else if (!strcmp(argv[1], "game_is_connected"))
          res = test_game_is_connected(g); */
  else if (!strcmp(argv[1], "game_is_connected"))
    res = test_game_is_connected() ? EXIT_SUCCESS : EXIT_FAILURE;
  else if (!strcmp(argv[1], "game_play_move"))
    res = test_game_play_move();
  else if (!strcmp(argv[1], "game_get_piece_shape"))
    res = test_game_get_piece_shape();
  else if (!strcmp(argv[1], "game_get_piece_orientation"))
    res = test_game_get_piece_orientation();
  else if (!strcmp(argv[1], "game_set_piece_orientation"))
    res = test_game_set_piece_orientation();
  else if (!strcmp(argv[1], "game_delete"))
    res = test_game_delete();
  else if (!strcmp(argv[1], "game_equal"))
    res = test_game_equal();
  else {
    fprintf(stderr, "Invalid test name: %s\n", argv[1]);
    return (EXIT_FAILURE);
  }
  if (res == EXIT_FAILURE)
    printf("test failure\n");
  else
    printf("test success\n");
  return (res);
}
