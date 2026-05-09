// SDL2 Demo by aurelien.esnard@u-bordeaux.fr

#include "model.h"

#include <SDL.h>
#include <SDL_image.h>  // required to load transparent texture from PNG
#include <SDL_ttf.h>    // required to use TTF fonts
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"

/* **************************************************************** */

#define FONT "res/retropix.ttf"
#define FONTSIZE_TITLE 35    // "Police size: NxM"
#define FONTSIZE_HELP 20     // "Police Press [h] for help" et aussi les information
#define GRID_COLOR 0xF9FF00  // Jaune (#f9ff00) la grille

struct Env_t {
  /* === Textures statiques === */
  SDL_Texture* background;  // Texture pour l'arrière-plan du jeu
  SDL_Texture* win;         // Texture pour l'affichage de la victoire

  /* === Textures des symboles de jeu === */
  SDL_Texture* corner;    // Texture pour les pièces de type "coin" (└, ┌, etc.)
  SDL_Texture* segment;   // Texture pour les segments droits (| ou -)
  SDL_Texture* cross;     // Texture pour les croix (┼)
  SDL_Texture* tee;       // Texture pour les T (┴, ├, etc.)
  SDL_Texture* endpoint;  // Texture pour les extrémités (^, >, etc.)

  /* === Gestion des polices et textes === */
  TTF_Font* font_title;     // Police pour le titre "size: NxM"
  TTF_Font* font_help;      // Police pour le texte d'aide
  SDL_Texture* text_help;   // Texture pré-rendue pour "Press [h] for help"
  char title_text[20];      // Buffer pour stocker dynamiquement "size: NxM"
  SDL_Texture* text_title;  // Texture pré-rendue pour le titre "size: NxM"

  /* === Gestion de la grille === */
  int grid_rows;      // Nombre de lignes de la grille
  int grid_cols;      // Nombre de colonnes de la grille
  int cell_width;     // Largeur d'une cellule en pixels
  int cell_height;    // Hauteur d'une cellule en pixels
  int grid_offset_x;  // Position X de départ de la grille (marge gauche)
  int grid_offset_y;  // Position Y de départ de la grille (marge haute)

  /* === Aide contextuelle === */
  SDL_Texture* help_text_tab;  // Texture pour le panneau d'aide complet
  char* help_message;          // Message d'aide complet (multi-lignes)

  /* === État du jeu === */
  game g;            // Structure principale du jeu (logique métier)
  bool show_help;    // Flag pour afficher/cacher l'aide
  bool game_won;     // Flag indiquant si le joueur a gagné
  bool show_errors;  // Flag pour afficher les erreurs de connexion
};

/* **************************************************************** */

// Cette fonction met à jour dynamiquement le texte affichant la taille de la grille (ex: "size: 5x5") dans l'interface graphique.
void update_title_text(Env* env, SDL_Renderer* ren)
{
  // Libérer l'ancienne texture si elle existe
  if (env->text_title) {
    SDL_DestroyTexture(env->text_title);
  }
  // Créer le nouveau texte
  snprintf(env->title_text, sizeof(env->title_text), "Size: %dx%d", env->grid_cols, env->grid_rows);
  // Recréer la texture
  SDL_Color color_pink = {0xFF, 0x75, 0xED, 255};
  env->font_title = TTF_OpenFont(FONT, FONTSIZE_TITLE);
  SDL_Surface* surf = TTF_RenderText_Blended(env->font_title, env->title_text, color_pink);
  env->text_title = SDL_CreateTextureFromSurface(ren, surf);
  SDL_FreeSurface(surf);
}
bool is_piece_connected(cgame g, uint i, uint j) {
  if (game_get_piece_shape(g, i, j) == EMPTY) 
      return false;
  
  // Check all 4 directions for MATCH connections
  for (direction d = NORTH; d <= WEST; d++) {
      if (game_check_edge(g, i, j, d) == MATCH) {
          return true;
      }
  }
  return false;
}
/* **************************************************************** */

Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[])
{
  Env* env = malloc(sizeof(struct Env_t));
  if (!env) return NULL;

  // Initialize ALL struct members
  memset(env, 0, sizeof(struct Env_t));
  if (argc > 1) {  // Si un fichier est fourni en argument
    env->g = game_load(argv[1]);
    if (!env->g) {
      fprintf(stderr, "Erreur: Impossible de charger le fichier '%s'. Utilisation du jeu par défaut.\n", argv[1]);
      env->g = game_default();
    }
  } else {  // Sinon, jeu par défaut
    env->g = game_default();
  }
  /* Charger les textures */
  env->background = IMG_LoadTexture(ren, "res/background1.png");
  env->win = IMG_LoadTexture(ren, "res/win.png");
  env->corner = IMG_LoadTexture(ren, "res/corner.png");
  env->segment = IMG_LoadTexture(ren, "res/segment.png");
  env->cross = IMG_LoadTexture(ren, "res/cross.png");
  env->tee = IMG_LoadTexture(ren, "res/tee.png");
  env->endpoint = IMG_LoadTexture(ren, "res/endpoint.png");

  /* Charger les polices */
  env->font_help = TTF_OpenFont(FONT, FONTSIZE_HELP);
  SDL_Color color_pink = {0xFF, 0x75, 0xED, 255};
  SDL_Surface* surf_help = TTF_RenderText_Blended(env->font_help, "Press [h] for help", color_pink);
  env->text_help = SDL_CreateTextureFromSurface(ren, surf_help);
  SDL_FreeSurface(surf_help);

  // Créer le texte d'aide
  env->help_message = strdup(
      "Click left or right buttons to play move:\n"
      "Press key:\n"
      "[n] : new random game\n"
      "[r] : restart (shuffle)\n"
      "[s] : solve\n"
      "[z] : undo\n"
      "[y] : redo\n"
      "[p] : print\n"
      "[e] : toggle errors\n"
      "[h] : help\n"
      "[q] : quit");

  SDL_Surface* help_surf = TTF_RenderText_Blended_Wrapped(env->font_help, env->help_message, color_pink,
                                                          800);  // Largeur max
  env->help_text_tab = SDL_CreateTextureFromSurface(ren, help_surf);
  SDL_FreeSurface(help_surf);

  /* Initialiser la grille */
  env->grid_rows = game_nb_rows(env->g);
  env->grid_cols = game_nb_cols(env->g);

  /* Calculer la taille des cellules */
  int window_width, window_height;
  SDL_GetWindowSize(win, &window_width, &window_height);

  int cell_size = (window_width - 100) / env->grid_cols;
  int cell_size_h = (window_height - 160) / env->grid_rows;
  cell_size = (cell_size < cell_size_h) ? cell_size : cell_size_h;  // Prendre le plus petit
  env->cell_width = env->cell_height = cell_size;                   // Garder la même taille

  /* Position de la grille */
  env->grid_offset_x = (window_width - env->grid_cols * env->cell_width) / 2;
  env->grid_offset_y = (window_height - env->grid_rows * env->cell_height) / 2;

  /* État initial */
  env->show_help = false;
  env->game_won = false;
  env->show_errors = false;
  update_title_text(env, ren);  // Size "NxM"
  return env;
}

/* **************************************************************** */

void render(SDL_Window* win, SDL_Renderer* ren, Env* env)
{
  if (!env || !ren) return;

  int win_width, win_height;
  SDL_GetWindowSize(win, &win_width, &win_height);

  /* Fond */
  SDL_RenderClear(ren);
  SDL_RenderCopy(ren, env->background, NULL, NULL);

  /* Texte "size: NxN" en haut */
  if (env->text_title) {
    int tex_w, tex_h;
    SDL_QueryTexture(env->text_title, NULL, NULL, &tex_w, &tex_h);

    SDL_Rect title_rect = {(win_width - tex_w) / 2, 20, tex_w, tex_h};
    SDL_RenderCopy(ren, env->text_title, NULL, &title_rect);
  }

  /* Dessiner la grille et les pièces */
  for (uint i = 0; i < env->grid_rows; i++) {
    for (uint j = 0; j < env->grid_cols; j++) {
      SDL_Rect cell_rect = {env->grid_offset_x + j * env->cell_width, env->grid_offset_y + i * env->cell_height, env->cell_width, env->cell_height};

      /* Dessiner la cellule */
      SDL_SetRenderDrawColor(ren, (GRID_COLOR >> 16) & 0xFF, (GRID_COLOR >> 8) & 0xFF, GRID_COLOR & 0xFF, 255);
      SDL_RenderDrawRect(ren, &cell_rect);

      /* Dessiner la pièce avec la bonne orientation */
      shape s = game_get_piece_shape(env->g, i, j);
      direction dir = game_get_piece_orientation(env->g, i, j);

      SDL_Texture* tex = NULL;
      switch (s) {
        case CORNER:
          tex = env->corner;
          break;
        case SEGMENT:
          tex = env->segment;
          break;
        case CROSS:
          tex = env->cross;
          break;
        case TEE:
          tex = env->tee;
          break;
        case ENDPOINT:
          tex = env->endpoint;
          break;
        default:
          continue;
      }

      if (tex) {
        /* Calculer l'angle de rotation en fonction de l'orientation */
        bool connected = is_piece_connected(env->g, i, j);
        if (!connected) {
          // Màu đen trắng (xám đậm)
          SDL_SetTextureColorMod(tex, 80, 80, 80);
          SDL_SetTextureAlphaMod(tex, 200);  // Làm mờ nhẹ
      } else {
          // Màu gốc (đầy đủ)
          SDL_SetTextureColorMod(tex, 255, 255, 255);
          SDL_SetTextureAlphaMod(tex, 255);
      }
        double angle = 0;
        switch (dir) {
          case EAST:
            angle = 90;
            break;
          case SOUTH:
            angle = 180;
            break;
          case WEST:
            angle = 270;
            break;
          case NORTH:
          default:
            angle = 0;
            break;
        }
        /* Dessiner avec rotation */
        SDL_Rect symbol_rect = {cell_rect.x + cell_rect.w * 0.1, cell_rect.y + cell_rect.h * 0.1, cell_rect.w * 0.8, cell_rect.h * 0.8};
        SDL_RenderCopyEx(ren, tex, NULL, &symbol_rect, angle, NULL, SDL_FLIP_NONE);
        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);
      }
    }
  }

  /* Texte "Press [h] for help" */
  if (env->text_help) {
    int tex_w, tex_h;
    SDL_QueryTexture(env->text_help, NULL, NULL, &tex_w, &tex_h);

    SDL_Rect help_rect = {(win_width - tex_w) / 2, win_height - 40, tex_w, tex_h};
    SDL_RenderCopy(ren, env->text_help, NULL, &help_rect);
  }

  /* Onglet d'aide (si show_help == true) */
  if (env->show_help) {
    // Calculate dynamic font size based on window height
    int help_font_size = win_height / 30;
    if (help_font_size < 12) help_font_size = 12;  // Minimum size
    if (help_font_size != TTF_FontHeight(env->font_help)) {
      // Recreate font if size changed
      TTF_CloseFont(env->font_help);
      env->font_help = TTF_OpenFont(FONT, help_font_size);
    }

    // Help background (60% of window size)
    int help_bg_w = win_width * 0.6;
    int help_bg_h = win_height * 0.6;
    SDL_Rect help_bg = {(win_width - help_bg_w) / 2, (win_height - help_bg_h) / 2, help_bg_w, help_bg_h};

    // Semi-transparent background
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 200);
    SDL_RenderFillRect(ren, &help_bg);

    // White border
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderDrawRect(ren, &help_bg);

    // Regenerate help text if needed
    if (!env->help_text_tab) {
      SDL_Color color_pink = {0xFF, 0x75, 0xED, 255};
      SDL_Surface* help_surf = TTF_RenderText_Blended_Wrapped(env->font_help, env->help_message, color_pink,
                                                              help_bg.w - 50);  // 50px margin
      env->help_text_tab = SDL_CreateTextureFromSurface(ren, help_surf);
      SDL_FreeSurface(help_surf);
    }

    // Render help text
    if (env->help_text_tab) {
      int tex_w, tex_h;
      SDL_QueryTexture(env->help_text_tab, NULL, NULL, &tex_w, &tex_h);
      SDL_Rect help_text_rect = {help_bg.x + 25, help_bg.y + 25, tex_w, tex_h};
      SDL_RenderCopy(ren, env->help_text_tab, NULL, &help_text_rect);
    }

    // OK button (bottom-right corner of help_bg)
    SDL_Rect ok_button = {help_bg.x + help_bg.w - 120, help_bg.y + help_bg.h - 50, 100, 40};
    SDL_SetRenderDrawColor(ren, 100, 100, 255, 255);
    SDL_RenderFillRect(ren, &ok_button);

    // Render "OK" text
    SDL_Color color_pink = {0xFF, 0x75, 0xED, 255};
    SDL_Surface* ok_surf = TTF_RenderText_Blended(env->font_help, "OK", color_pink);
    SDL_Texture* ok_text = SDL_CreateTextureFromSurface(ren, ok_surf);
    SDL_Rect ok_text_rect = {ok_button.x + (ok_button.w - ok_surf->w) / 2, ok_button.y + (ok_button.h - ok_surf->h) / 2, ok_surf->w, ok_surf->h};
    SDL_RenderCopy(ren, ok_text, NULL, &ok_text_rect);
    SDL_FreeSurface(ok_surf);
    SDL_DestroyTexture(ok_text);
  }

  /* Annonce "win" (si game_won == true) */
  if (env->game_won) {
    static Uint32 start_time = 0;
    static bool animation_done = false;

    if (!animation_done) {
      start_time = SDL_GetTicks();
      animation_done = true;
    }

    Uint32 current_time = SDL_GetTicks() - start_time;
    float progress = (current_time % 2000) / 2000.0f;  // Boucle toutes les 2 secondes

    // Effet de pulsation très prononcé (taille oscille entre 80% et 100% de la plus petite dimension)
    float scale = 0.9f + 0.1f * sin(progress * 2 * M_PI);  // Taille énorme avec pulsation

    // Taille de base très grande (90% de la plus petite dimension de la fenêtre)
    int base_size = (win_width < win_height ? win_width : win_height) * 0.9f;
    int announcement_size = (int)(base_size * scale);

    // Positionnement centré avec débordement contrôlé
    SDL_Rect win_rect = {(win_width - announcement_size) / 2, (win_height - announcement_size) / 2, announcement_size, announcement_size};

    // Effet de couleur et transparence
    SDL_SetTextureAlphaMod(env->win, (Uint8)(230 + 25 * sin(progress * 4 * M_PI)));  // Clignotement rapide
    SDL_RenderCopy(ren, env->win, NULL, &win_rect);
  }

  /* Affichage des erreurs de connexion entre les pièces */
  if (env->show_errors) {
    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);  // rouge
    const int border_thickness = 3;               // Épaisseur des bordures d'erreur

    for (uint i = 0; i < env->grid_rows; i++) {
      for (uint j = 0; j < env->grid_cols; j++) {
        if (game_get_piece_shape(env->g, i, j) == EMPTY) continue;

        // Coordonnées de la cellule
        int cell_x = env->grid_offset_x + j * env->cell_width;
        int cell_y = env->grid_offset_y + i * env->cell_height;

        /* Vérification des 4 directions possibles pour détecter les mauvaises connexions */
        for (direction d = NORTH; d <= WEST; d++) {
          if (game_check_edge(env->g, i, j, d) == MISMATCH) {
            switch (d) {
              case NORTH: {  // Bord supérieur mal connecté
                // Dessine une ligne rouge en haut de la cellule
                SDL_Rect top_border = {cell_x, cell_y - border_thickness / 2, env->cell_width, border_thickness};
                SDL_RenderFillRect(ren, &top_border);
                break;
              }
              case EAST: {  // Bord droit mal connecté
                // Dessine une ligne rouge à droite de la cellule
                SDL_Rect right_border = {cell_x + env->cell_width - border_thickness / 2, cell_y, border_thickness, env->cell_height};
                SDL_RenderFillRect(ren, &right_border);
                break;
              }
              case SOUTH: {  // Bord inférieur mal connecté
                // Dessine une ligne rouge en bas de la cellule
                SDL_Rect bottom_border = {cell_x, cell_y + env->cell_height - border_thickness / 2, env->cell_width, border_thickness};
                SDL_RenderFillRect(ren, &bottom_border);
                break;
              }
              case WEST: {  // Bord gauche mal connecté
                // Dessine une ligne rouge à gauche de la cellule
                SDL_Rect left_border = {cell_x - border_thickness / 2, cell_y, border_thickness, env->cell_height};
                SDL_RenderFillRect(ren, &left_border);
                break;
              }
              default:
                break;  // Cas normalement impossible
            }
          }
        }
      }
    }
  }
}

/* **************************************************************** */

bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e)
{
  if (e->type == SDL_QUIT) {
    return true;
  }

  int win_width, win_height;
  SDL_GetWindowSize(win, &win_width, &win_height);

  if (e->type == SDL_WINDOWEVENT) {
    if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
      int new_width = e->window.data1;
      int new_height = e->window.data2;

      // Calculate max cell size to fit within the window while keeping cells square
      int margin_x = 50;  // Left and right margins
      int margin_y = 80;  // Top and bottom margins

      int cell_size = (new_width - 2 * margin_x) / env->grid_cols;
      int cell_size_h = (new_height - 2 * margin_y) / env->grid_rows;
      cell_size = (cell_size < cell_size_h) ? cell_size : cell_size_h;
      env->cell_width = env->cell_height = cell_size;

      // Center the grid within the new window dimensions
      env->grid_offset_x = (new_width - env->grid_cols * env->cell_width) / 2;
      env->grid_offset_y = (new_height - env->grid_rows * env->cell_height) / 2;
      if (env->show_help && env->help_text_tab) {
        SDL_DestroyTexture(env->help_text_tab);
        env->help_text_tab = NULL;  // Will regenerate in render()
      }
    }
  }

  if (env->show_help && e->type == SDL_MOUSEBUTTONDOWN) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    // Calcul identique à celui dans render()
    int help_bg_w = win_width * 0.6;
    int help_bg_h = win_height * 0.6;
    SDL_Rect ok_button = {(win_width - help_bg_w) / 2 + help_bg_w - 120, (win_height - help_bg_h) / 2 + help_bg_h - 50, 100, 40};

    if (mouse_x >= ok_button.x && mouse_x <= ok_button.x + ok_button.w && mouse_y >= ok_button.y && mouse_y <= ok_button.y + ok_button.h) {
      env->show_help = false;
    }
  }

  if (e->type == SDL_KEYDOWN) {
    switch (e->key.keysym.sym) {
      case SDLK_h:
        env->show_help = !env->show_help;
        break;
      case SDLK_q:
        return true;
      case SDLK_z:
        game_undo(env->g);
        break;
      case SDLK_y:
        game_redo(env->g);
        break;
      case SDLK_e:
        env->show_errors = !env->show_errors;
        break;
      case SDLK_s:
        game_delete(env->g);
        env->g = game_default_solution();
        env->game_won = true;
        break;
      case SDLK_r:
        game_shuffle_orientation(env->g);
        break;
      case SDLK_p:
        game_print(env->g);
        break;
      case SDLK_n:
        game_delete(env->g);

        // Generate random grid size (3x3 to 10x10 example)
        env->grid_rows = (rand() % 8) + 3;  // 3-10 rows
        env->grid_cols = (rand() % 8) + 3;  // 3-10 cols

        // Calculate cell size to fit CURRENT window
        int margin_x = 50;  // Horizontal margin
        int margin_y = 80;  // Vertical margin
        int max_cell_width = (win_width - 2 * margin_x) / env->grid_cols;
        int max_cell_height = (win_height - 2 * margin_y) / env->grid_rows;
        env->cell_width = env->cell_height = (max_cell_width < max_cell_height) ? max_cell_width : max_cell_height;

        // Center the grid within CURRENT window
        env->grid_offset_x = (win_width - env->grid_cols * env->cell_width) / 2;
        env->grid_offset_y = (win_height - env->grid_rows * env->cell_height) / 2;

        // Generate new random game with minimal empty cells but not solved
        uint nb_empty = (rand() % 3);      // 0 à 2 cases vides maximum
        uint nb_extra = (rand() % 3) + 1;  // 1 à 3 arêtes supplémentaires pour désordonner

        env->g = game_random(env->grid_rows, env->grid_cols, false, nb_empty, nb_extra);

        // Shuffle orientations to ensure it's not solved
        game_shuffle_orientation(env->g);

        env->game_won = false;
        update_title_text(env, ren);
        break;
    }
  }
  if (e->type == SDL_MOUSEBUTTONDOWN) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    /* Vérifier si clic dans la grille */
    if (mouse_x >= env->grid_offset_x && mouse_x <= env->grid_offset_x + env->grid_cols * env->cell_width && mouse_y >= env->grid_offset_y &&
        mouse_y <= env->grid_offset_y + env->grid_rows * env->cell_height) {
      uint cell_i = (mouse_y - env->grid_offset_y) / env->cell_height;
      uint cell_j = (mouse_x - env->grid_offset_x) / env->cell_width;

      if (cell_i < env->grid_rows && cell_j < env->grid_cols) {
        if (e->button.button == SDL_BUTTON_LEFT) {
          // Rotation anti-horaire
          game_play_move(env->g, cell_i, cell_j, -1);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
          // Rotation horaire
          game_play_move(env->g, cell_i, cell_j, 1);
        }

        // Vérifier si le jeu est gagné
        env->game_won = game_won(env->g);
      }
    }
  }
  return false;
}

/* **************************************************************** */

void clean(SDL_Window* win, SDL_Renderer* ren, Env* env)
{
  if (!env) return;

  /* === Libération des textures statiques === */
  if (env->background) SDL_DestroyTexture(env->background);
  if (env->win) SDL_DestroyTexture(env->win);

  /* === Libération des textures des symboles === */
  if (env->corner) SDL_DestroyTexture(env->corner);
  if (env->segment) SDL_DestroyTexture(env->segment);
  if (env->cross) SDL_DestroyTexture(env->cross);
  if (env->tee) SDL_DestroyTexture(env->tee);
  if (env->endpoint) SDL_DestroyTexture(env->endpoint);

  /* === Libération des textures texte === */
  if (env->text_title) SDL_DestroyTexture(env->text_title);
  if (env->text_help) SDL_DestroyTexture(env->text_help);
  if (env->help_text_tab) SDL_DestroyTexture(env->help_text_tab);

  /* === Libération de la mémoire allouée pour les messages === */
  if (env->help_message) free(env->help_message);

  /* === Libération des polices === */
  if (env->font_title) TTF_CloseFont(env->font_title);
  if (env->font_help) TTF_CloseFont(env->font_help);

  /* === Libération de la structure de jeu === */
  if (env->g) game_delete(env->g);

  /* === Libération finale de la structure Env === */
  free(env);
}

/* **************************************************************** */