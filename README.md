# NET-C29

A C implementation of the **NET** puzzle game, with several playable versions:

- a terminal version,
- a random grid generator,
- a puzzle solver,
- an SDL2 graphical interface,
- a WebAssembly/browser version.

## Authors

- Toan CAO
- My Hanh DINH

## Game Description

**NET** is a single-player logic puzzle played on a rectangular grid.  
Each cell may contain a piece representing a part of a network:

- endpoint,
- segment,
- corner,
- tee,
- cross,
- empty cell.

Each piece has an orientation: North, East, South, or West.  
The player can rotate the pieces until all connections are correctly matched.

The goal is to connect all non-empty pieces into a valid network. In this version of the project:

- grids may contain empty cells;
- solutions with loops are accepted;
- both regular and wrapping grids are supported.

In wrapping mode, the top edge is connected to the bottom edge, and the left edge is connected to the right edge.

## Project Structure

```text
net-c29/
├── README.md
├── SDL/
│   ├── game.c
│   ├── game_aux.c
│   ├── game_ext.c
│   ├── game_tools.c
│   ├── queue.c
│   ├── game_text.c
│   ├── game_random.c
│   ├── game_solve.c
│   ├── game_sdl.c
│   ├── model.c
│   ├── CMakeLists.txt
│   ├── ex/
│   │   ├── default.txt
│   │   ├── default.sol
│   │   ├── game11.txt
│   │   ├── game11.sol1
│   │   └── game11.sol2
│   └── res/
│       └── images and fonts used by the SDL interface
│
└── pt2-jeu-o7gd1y/
    ├── game.html
    ├── game_canvas.js
    ├── game_style.css
    ├── game.js
    ├── game.wasm
    ├── wrapper.c
    └── src/
```

## Features

### Core Game

- rectangular grids;
- support for empty cells;
- support for wrapping and non-wrapping modes;
- piece rotation;
- game loading and saving;
- victory detection;
- undo and redo;
- random grid generation;
- puzzle solving;
- counting solutions.

### SDL Graphical Version

The SDL version provides a visual and interactive interface with:

- mouse-based piece rotation;
- resizable window;
- graphical assets for all piece types;
- undo and redo;
- random game generation;
- error highlighting;
- win screen;
- keyboard shortcuts.

### Web Version

The browser version is based on WebAssembly and HTML canvas. It includes:

- random game generation;
- selectable grid sizes;
- custom grid sizes from 2×2 to 20×20;
- wrapping mode toggle;
- undo and redo;
- error checking;
- automatic solving;
- responsive canvas display.

## Requirements

### Linux

Install the required packages:

```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### macOS

Using Homebrew:

```bash
brew install cmake sdl2 sdl2_image sdl2_ttf
```

## Build Instructions

The C and SDL programs are built from the `SDL/` directory.

```bash
cd SDL
mkdir -p build
cd build
cmake ..
make
```

After compilation, the following executables are generated:

```text
game_text
game_random
game_solve
game_sdl
```

## Running the Programs

### SDL Graphical Interface

Run the graphical version with the default game:

```bash
./game_sdl
```

Run it with a specific grid file:

```bash
./game_sdl ex/game11.txt
```

### Terminal Version

Run the text version with the default game:

```bash
./game_text
```

Run it with a grid file:

```bash
./game_text ex/game11.txt
```

Available commands in terminal mode:

```text
h             show help
c i j         rotate the piece at row i, column j clockwise
a i j         rotate the piece at row i, column j counter-clockwise
r             shuffle the current game
z             undo the last move
y             redo the last undone move
s filename    save the current game to a file
q             quit
```

### Random Game Generator

Usage:

```bash
./game_random <nb_rows> <nb_cols> <wrapping> <nb_empty> <nb_extra> <shuffle> [filename]
```

Example:

```bash
./game_random 5 5 0 2 1 1 random_game.txt
```

Arguments:

```text
nb_rows     number of rows
nb_cols     number of columns
wrapping    0 for false, 1 for true
nb_empty    number of empty cells
nb_extra    number of extra edges
shuffle     0 for false, 1 for true
filename    optional output file
```

### Solver

Solve a grid and save the solved game:

```bash
./game_solve -s ex/game11.txt solved_game.txt
```

Count the number of solutions and save the result:

```bash
./game_solve -c ex/game11.txt number_of_solutions.txt
```

## SDL Controls

### Mouse

```text
Left click      rotate a piece counter-clockwise
Right click     rotate a piece clockwise
```

### Keyboard

```text
n       generate a new random game
r       restart / shuffle the current game
s       display a solved default board
z       undo
y       redo
p       print the current game in the terminal
e       toggle error highlighting
h       show or hide help
q       quit
```

When error highlighting is enabled, incorrect or unmatched connections are displayed in red.

## Web Version

The web version is located in:

```text
pt2-jeu-o7gd1y/
```

To run it locally, start a small local web server:

```bash
cd pt2-jeu-o7gd1y
python3 -m http.server 8000
```

Then open:

```text
http://localhost:8000/game.html
```

The main web game file is:

```text
game.html
```

The browser version provides buttons for:

```text
New Game
Restart
Undo
Redo
Check
Solve
Wrapping ON/OFF
```

It also supports predefined grid sizes and custom grids between 2×2 and 20×20.

## Tests

The repository contains several test files:

```text
game_test_mdinh.c
game_test_tcao002.c
game_test_yelmensi.c
```

A basic manual solver test can be run after compilation:

```bash
./game_solve -s ex/game11.txt game11_output.txt
```

You can also compare the generated output with the provided solution files in `ex/`.

If CTest is configured correctly, tests can be launched from the build directory with:

```bash
ctest --output-on-failure
```

## File Format

Game files describe a grid and the pieces it contains. Example files are available in:

```text
SDL/ex/
```

Useful examples:

```text
default.txt
default.sol
game11.txt
game11.sol1
game11.sol2
```

These files can be loaded by `game_text`, `game_sdl`, and `game_solve`.