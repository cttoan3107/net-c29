cmake_minimum_required(VERSION 3.5)
project(net-c29 LANGUAGES C)

include(CTest)
enable_testing()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-std=c99 -g -Wall --coverage")


# Trouver SDL2
include(sdl2.cmake)
message(STATUS "SDL2 include dir: ${SDL2_ALL_INC}")
message(STATUS "SDL2 all libraries: ${SDL2_ALL_LIBS}")

# Ajouter la bibliothèque principale du jeu
add_library(game game.c game_aux.c game_ext.c queue.c game_tools.c game_private.c)

# Exécutables pour les outils en ligne de commande
add_executable(game_text game_text.c)
add_executable(game_random game_random.c)
add_executable(game_solve game_solve.c)

# Lier les exécutables à la bibliothèque game
target_link_libraries(game_text game)
target_link_libraries(game_random game)
target_link_libraries(game_solve game)

# Copier les fichiers de grille par défaut dans le répertoire de construction
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ex DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Ajouter un test pour game_solve
add_test(test_game_solve ./game_solve -s game11.txt game11.sol1)
set_tests_properties(test_game_solve PROPERTIES TIMEOUT 10)

# Ajouter l'exécutable game_sdl
include_directories(${SDL2_ALL_INC})

add_executable(game_sdl game_sdl.c model.c)
target_link_libraries(game_sdl game ${SDL2_ALL_LIBS} m )

# Copier les ressources (images, polices, etc.) dans le répertoire de construction
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})