
#include "game_tools.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#define MAX_ROTATIONS 4

// @copyright University of Bordeaux. All rights reserved, 2024.
/* ************************************************************************** */
/** @brief Hard-coding of pieces (shape & orientation) in an integer array.
 * @details The 4 least significant bits encode the presence of an half-edge in
 * the N-E-S-W directions (in that order). Thus, binary coding 1100 represents
 * the piece "└" (a corner in north orientation).
 */
static uint _code[NB_SHAPES][NB_DIRS] = {
    {0b0000, 0b0000, 0b0000, 0b0000},  // EMPTY {" ", " ", " ", " "}
    {0b1000, 0b0100, 0b0010, 0b0001},  // ENDPOINT {"^", ">", "v", "<"},
    {0b1010, 0b0101, 0b1010, 0b0101},  // SEGMENT {"|", "-", "|", "-"},
    {0b1100, 0b0110, 0b0011, 0b1001},  // CORNER {"└", "┌", "┐", "┘"}
    {0b1101, 0b1110, 0b0111, 0b1011},  // TEE {"┴", "├", "┬", "┤"}
    {0b1111, 0b1111, 0b1111, 0b1111}   // CROSS {"+", "+", "+", "+"}
};
/* ************************************************************************** */
/** encode a shape and an orientation into an integer code */
static uint _encode_shape(shape s, direction o) { return _code[s][o]; }
/* ************************************************************************** */
/** decode an integer code into a shape and an orientation */
static bool _decode_shape(uint code, shape* s, direction* o)
{
  assert(code >= 0 && code < 16);
  assert(s);
  assert(o);
  for (int i = 0; i < NB_SHAPES; i++)
    for (int j = 0; j < NB_DIRS; j++)
      if (code == _code[i][j]) {
        *s = i;
        *o = j;
        return true;
      }
  return false;
}
/* ************************************************************************** */
/** add an half-edge in the direction d */
static void _add_half_edge(game g, uint i, uint j, direction d)
{
  assert(g);
  assert(i < game_nb_rows(g));
  assert(j < game_nb_cols(g));
  assert(d < NB_DIRS);
  shape s = game_get_piece_shape(g, i, j);
  direction o = game_get_piece_orientation(g, i, j);
  uint code = _encode_shape(s, o);
  uint mask = 0b1000 >> d;     // mask with half-edge in the direction d
  assert((code & mask) == 0);  // check there is no half-edge in the direction d
  uint newcode = code | mask;  // add the half-edge in the direction d
  shape news;
  direction newo;
  bool ok = _decode_shape(newcode, &news, &newo);
  assert(ok);
  game_set_piece_shape(g, i, j, news);
  game_set_piece_orientation(g, i, j, newo);
}
/* ************************************************************************** */
#define OPPOSITE_DIR(d) ((d + 2) % NB_DIRS)
/* ************************************************************************** */
/**
 * @brief Add an edge between two adjacent squares.
 * @details This is done by modifying the pieces of the two adjacent squares.
 * More precisely, we add an half-edge to each adjacent square, so as to build
 * an edge between these two squares.
 * @param g the game
 * @param i row index
 * @param j column index
 * @param d the direction of the adjacent square
 * @pre @p g must be a valid pointer toward a game structure.
 * @pre @p i < game height
 * @pre @p j < game width
 * @return true if an edge can be added, false otherwise
 */
static bool _add_edge(game g, uint i, uint j, direction d)
{
  assert(g);
  assert(i < game_nb_rows(g));
  assert(j < game_nb_cols(g));
  assert(d < NB_DIRS);
  uint nexti, nextj;
  bool next = game_get_ajacent_square(g, i, j, d, &nexti, &nextj);
  if (!next) return false;
  // check if the two half-edges are free
  bool he = game_has_half_edge(g, i, j, d);
  if (he) return false;
  bool next_he = game_has_half_edge(g, nexti, nextj, OPPOSITE_DIR(d));
  if (next_he) return false;
  _add_half_edge(g, i, j, d);
  _add_half_edge(g, nexti, nextj, OPPOSITE_DIR(d));
  return true;
}
/****************************************************************************************************/
/* ************************************************************************** */
game game_load(char* filename)
{
  if (filename == NULL) {
    fprintf(stderr, "Erreur d'existe de fichier\n");
    return NULL;
  }
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Erreur lors de l'ouverture du fichier\n");
    return NULL;
  }
  uint nb_rows, nb_cols;
  int wrapping;
  if (fscanf(f, "%u %u %d\n", &nb_rows, &nb_cols, &wrapping) != 3) {
    fprintf(stderr, "Erreur lors de la lecture des dimensions\n");
    fclose(f);
    return NULL;
  }
  game g = game_new_empty_ext(nb_rows, nb_cols, wrapping);
  if (g == NULL) {
    fclose(f);
    return NULL;
  }
  char shape, dir;
  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      if (fscanf(f, " %c%c", &shape, &dir) != 2) {
        fprintf(stderr, "failed to read shape and orientation\n");
        fclose(f);
        game_delete(g);  // Ajout : libérer la mémoire en cas d'erreur
        return NULL;
      }
      switch (shape) {
        case 'E':
          game_set_piece_shape(g, i, j, EMPTY);
          break;
        case 'N':
          game_set_piece_shape(g, i, j, ENDPOINT);
          break;
        case 'S':
          game_set_piece_shape(g, i, j, SEGMENT);
          break;
        case 'C':
          game_set_piece_shape(g, i, j, CORNER);
          break;
        case 'T':
          game_set_piece_shape(g, i, j, TEE);
          break;
        case 'X':
          game_set_piece_shape(g, i, j, CROSS);
          break;
        default:
          fprintf(stderr, "Invalid shape in file\n");
          fclose(f);
          game_delete(g);
          return NULL;
      }
      switch (dir) {
        case 'N':
          game_set_piece_orientation(g, i, j, NORTH);
          break;
        case 'E':
          game_set_piece_orientation(g, i, j, EAST);
          break;
        case 'S':
          game_set_piece_orientation(g, i, j, SOUTH);
          break;
        case 'W':
          game_set_piece_orientation(g, i, j, WEST);
          break;
        default:
          fprintf(stderr, "Invalid direction in file\n");
          fclose(f);
          game_delete(g);
          return NULL;
      }
    }
  }
  fclose(f);
  return g;
}
void game_save(cgame g, char* filename)
{
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file for writing\n");
    return;
  }
  fprintf(file, "%u %u %d\n", g->nb_rows, g->nb_cols, g->wrapping);
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      char shape = ' ';
      switch (game_get_piece_shape(g, i, j)) {
        case EMPTY:
          shape = 'E';
          break;
        case ENDPOINT:
          shape = 'N';
          break;
        case SEGMENT:
          shape = 'S';
          break;
        case CORNER:
          shape = 'C';
          break;
        case TEE:
          shape = 'T';
          break;
        case CROSS:
          shape = 'X';
          break;
        default:
          break;
      }
      char direction = ' ';
      switch (game_get_piece_orientation(g, i, j)) {
        case NORTH:
          direction = 'N';
          break;
        case EAST:
          direction = 'E';
          break;
        case SOUTH:
          direction = 'S';
          break;
        case WEST:
          direction = 'W';
          break;
        default:
          break;
      }
      fprintf(file, "%c%c ", shape, direction);
    }
    fprintf(file, "\n");
  }
  fclose(file);
}
// Repeat if the cell is empty
void get_random_non_null_cell(game g, uint* row, uint* col, uint nb_rows, uint nb_cols)
{
  uint attempts = 0;
  const uint max_attempts = nb_rows * nb_cols * 10;  // Limit the number of retries
  do {
    *row = rand() % nb_rows;
    *col = rand() % nb_cols;
    attempts++;
    if (game_get_piece_shape(g, *row, *col) != EMPTY) {
      return;  // Valid non-empty cell found
    }
    if (attempts > max_attempts) {
      // If we exceed the max attempts, just select the first non-empty cell
      for (uint i = 0; i < nb_rows; i++) {
        for (uint j = 0; j < nb_cols; j++) {
          if (game_get_piece_shape(g, i, j) != EMPTY) {
            *row = i;
            *col = j;
            return;
          }
        }
      }
      // If all cells are empty, pick any random cell
      *row = rand() % nb_rows;
      *col = rand() % nb_cols;
      return;
    }
  } while (attempts <= max_attempts);
}
game game_random(uint nb_rows, uint nb_cols, bool wrapping, uint nb_empty, uint nb_extra)
{
  // Vérification du nombre de cellules vides
  if (nb_empty >= nb_rows * nb_cols) {
    fprintf(stderr, "Error: nb_empty is too large for the grid size.\n");
    return NULL;
  }
  game g = game_new_empty_ext(nb_rows, nb_cols, wrapping);
  if (!g) return NULL;
  uint start_i = rand() % nb_rows;
  uint start_j = rand() % nb_cols;
  uint d = rand() % NB_DIRS;
  uint pi_next, pj_next;
  if (game_get_ajacent_square(g, start_i, start_j, d, &pi_next, &pj_next)) {
    game_set_piece_shape(g, start_i, start_j, ENDPOINT);
    game_set_piece_orientation(g, start_i, start_j, d);
    game_set_piece_shape(g, pi_next, pj_next, ENDPOINT);
    game_set_piece_orientation(g, pi_next, pj_next, (d + 2) % NB_DIRS);
  }
  uint pieces_added = 2;
  while (pieces_added < nb_rows * nb_cols - nb_empty) {
    uint row, col;
    get_random_non_null_cell(g, &row, &col, nb_rows, nb_cols);
    d = rand() % NB_DIRS;
    if (game_get_ajacent_square(g, row, col, d, &pi_next, &pj_next)) {
      if (game_get_piece_shape(g, pi_next, pj_next) != EMPTY) {
        continue;
      }
      bool boolean = _add_edge(g, row, col, d);
      if (boolean) {
        game_set_piece_shape(g, pi_next, pj_next, ENDPOINT);
        game_set_piece_orientation(g, pi_next, pj_next, (d + 2) % NB_DIRS);
        pieces_added++;
      }
    }
  }
  uint i = 0;
  while (i < nb_extra) {
    uint row, col;
    get_random_non_null_cell(g, &row, &col, nb_rows, nb_cols);
    d = rand() % NB_DIRS;
    if (game_get_ajacent_square(g, row, col, d, &pi_next, &pj_next)) {
      if (game_get_piece_shape(g, pi_next, pj_next) == EMPTY) {
        continue;
      }
      bool boolean = _add_edge(g, row, col, d);
      if (boolean) i++;
    }
  }
  return g;
}
bool solve_recursive(game g, int cell_index, uint* count, bool early_stop)
{
  if (game_won(g)) {
    if (count) (*count)++;
    return true;
  }
  if (cell_index >= game_nb_rows(g) * game_nb_cols(g)) {
    return false;
  }
  uint i = cell_index / game_nb_cols(g);
  uint j = cell_index % game_nb_cols(g);
  shape s = game_get_piece_shape(g, i, j);

  direction original_orientation = game_get_piece_orientation(g, i, j);
  if (s == EMPTY) {
    return solve_recursive(g, cell_index + 1, count, early_stop);
  }
  int max_orientations = (s == SEGMENT) ? 2 : (s == CROSS) ? 1 : 4;
  for (int orientation = 0; orientation < max_orientations; orientation++) {
    game_set_piece_orientation(g, i, j, orientation);
    if (game_won(g)) {
      if (count) (*count)++;
      return true;
    }
    bool edges_valid = true;
    if ((i > 0 || g->wrapping == false) && game_check_edge(g, i, j, NORTH) == MISMATCH) {
      edges_valid = false;
    }
    if ((j > 0 || g->wrapping == false) && game_check_edge(g, i, j, WEST) == MISMATCH) {
      edges_valid = false;
    }

    if (edges_valid) {
      if (solve_recursive(g, cell_index + 1, count, early_stop)) {
        if (early_stop) {
          return true;
        }
      }
    }
  }
  game_set_piece_orientation(g, i, j, original_orientation);
  return false;
}

uint game_nb_solutions(cgame g)
{
  uint count = 0;
  game tmp = game_copy(g);
  solve_recursive(tmp, 0, &count, false);
  game_delete(tmp);
  return count;
}

bool game_solve(game g)
{
  bool solved = solve_recursive(g, 0, NULL, true);
  return solved;
}