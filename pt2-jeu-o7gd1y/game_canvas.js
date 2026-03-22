const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

// Variables globales
let game = null;
let winAnimationStart = null;
let showErrors = false;
let wrappingEnabled = true;

// Chargement des images
let winImage = new Image();
winImage.src = "photos/win.png";

const images = {
  1: new Image(), 
  2: new Image(), 
  3: new Image(),
  4: new Image(), 
  5: new Image(), 
};

images[1].src = "photos/endpoint.png";
images[2].src = "photos/segment.png";
images[3].src = "photos/corner.png";
images[4].src = "photos/tee.png";
images[5].src = "photos/cross.png";

Module.onRuntimeInitialized = () => {
  start();
};

function start() {
  console.log("WASM module loaded, creating default game");

  game = Module._new_default();
  drawGrid(game);
}

// Dessine la grille du jeu
function drawGrid(g) {
  const rows = Module._nb_rows(g);
  const cols = Module._nb_cols(g);

  const cellSize = Math.floor(Math.min(canvas.width / cols, canvas.height / rows));
  const offsetX = (canvas.width - cellSize * cols) / 2;
  const offsetY = (canvas.height - cellSize * rows) / 2;

  ctx.clearRect(0, 0, canvas.width, canvas.height);

  ctx.save();
  ctx.lineWidth = cellSize * 0.03;
  ctx.strokeStyle = "yellow";
  ctx.strokeRect(offsetX, offsetY, cols * cellSize, rows * cellSize);
  ctx.restore();

  for (let i = 0; i < rows; i++) {
    for (let j = 0; j < cols; j++) {
      const x = offsetX + j * cellSize;
      const y = offsetY + i * cellSize;

      ctx.strokeStyle = "yellow";
      ctx.strokeRect(x, y, cellSize, cellSize);

      const shape = Module._get_piece_shape(g, i, j);
      const orient = Module._get_piece_orientation(g, i, j);
      drawPiece(x + cellSize / 2, y + cellSize / 2, cellSize * 0.4, shape, orient);
    }
  }

  if (showErrors) {
    drawConnectionErrors(g, offsetX, offsetY, cellSize);
  }  
  document.getElementById("grid-size").textContent = `Size: ${rows} × ${cols}`;

}

// Dessine une pièce à une position donnée (avec rotation)
function drawPiece(cx, cy, size, shape, orient) {
  const img = images[shape];
  if (!img || !img.complete) return;

  ctx.save();
  ctx.translate(cx, cy);
  ctx.rotate((Math.PI / 2) * orient);

  const s = size;
  ctx.drawImage(img, -s, -s, 2 * s, 2 * s);

  ctx.restore();
}

// Interaction clic sur le canvas
canvas.addEventListener("click", function (event) {
    if (!game) return;
    showErrors = false;

    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
  
    const x = (event.clientX - rect.left) * scaleX;
    const y = (event.clientY - rect.top) * scaleY;

    const rows = Module._nb_rows(game);
    const cols = Module._nb_cols(game);
    const cellSize = Math.floor(Math.min(canvas.width / cols, canvas.height / rows));
    const offsetX = (canvas.width - cellSize * cols) / 2;
    const offsetY = (canvas.height - cellSize * rows) / 2;
  
    const j = Math.floor((x - offsetX) / cellSize);
    const i = Math.floor((y - offsetY) / cellSize);
  
    if (i < 0 || i >= rows || j < 0 || j >= cols) return;
  
    Module._play_move(game, i, j, 1);
    drawGrid(game);
  
    const hasWon = Module._won(game);
    if (hasWon) {
      winAnimationStart = performance.now();
      requestAnimationFrame(drawWinAnimation);
      setTimeout(() => {
        winAnimationStart = null;
        drawGrid(game);
      }, 2000);
    }
  });
  

 // Génère une nouvelle grille aléatoire
function onNewGame() {
  winAnimationStart = null;
  showErrors = false; 
  if (game) Module._delete(game);

  const size = 2 + Math.floor(Math.random() * 18); //limit de 2 à 20 pour sovle fonctionne bien 
  const wrapping = wrappingEnabled ? 1 : 0;
  const empty = 0;
  const extra = 0;

  game = Module._new_random(size, size, wrapping, empty, extra);
  Module._restart(game);
  drawGrid(game);
}  

// Mélange la grille actuelle
function onRestart() {
  winAnimationStart = null;
  showErrors = false;
  if (!game) return;
  Module._restart(game);
  drawGrid(game);
}

// Annule le dernier coup
function onUndo() {
  winAnimationStart = null;
  showErrors = false;
  if (!game) return;
  Module._undo(game);
  drawGrid(game);
}

// Refait un coup annulé
function onRedo() {
  winAnimationStart = null;
  showErrors = false;
  if (!game) return;
  Module._redo(game);
  drawGrid(game);
}

// Active/désactive l'affichage des erreurs de connexion
function onCheck() {
  showErrors = !showErrors;
  drawGrid(game);
}  

// Résout automatiquement le puzzle
function onSolve() {
  if (!game) return;
  showErrors = false;
  Module._set_wrapping(game, wrappingEnabled ? 1 : 0);
  const solved = Module._solve(game);
  drawGrid(game);
  if (solved) {
    winAnimationStart = performance.now();
    requestAnimationFrame(drawWinAnimation);
    setTimeout(() => {
      winAnimationStart = null;
      drawGrid(game);
    }, 2000);
  }
}

// Affiche une animation pulsante de victoire avec win.png
function drawWinAnimation(timestamp) {
  if (!winAnimationStart) return;

  const duration = 2000;
  const elapsed = (timestamp - winAnimationStart) % duration;
  const progress = elapsed / duration;

  const canvasWidth = canvas.width;
  const canvasHeight = canvas.height;
  const minDim = Math.min(canvasWidth, canvasHeight);

  const scale = 0.9 + 0.1 * Math.sin(progress * 2 * Math.PI);
  const baseSize = minDim * 0.9;
  const size = baseSize * scale;

  const alpha = 230 + 25 * Math.sin(progress * 4 * Math.PI);

  ctx.save();
  ctx.globalAlpha = alpha / 255;
  ctx.drawImage(winImage, (canvasWidth - size) / 2, (canvasHeight - size) / 2, size, size);
  ctx.restore();

  requestAnimationFrame(drawWinAnimation);
}

// un box aide caché
function toggleHelp() {
  const box = document.getElementById("help-box");
  if (box) {
    box.classList.toggle("hidden");
  }
}

// Touche clavier 'h' pour afficher le box aide
window.addEventListener("keydown", function (e) {
  if (e.key.toLowerCase() === "h") {
    toggleHelp();
  }
});

// Dessine les lignes rouges sur les connexions incorrectes
function drawConnectionErrors(g, offsetX, offsetY, cellSize) {
  const rows = Module._nb_rows(g);
  const cols = Module._nb_cols(g);
  const thickness = cellSize * 0.02;

  ctx.save();
  ctx.strokeStyle = "red";
  ctx.lineWidth = thickness;
  Module._set_wrapping(g, wrappingEnabled ? 1 : 0);

  for (let i = 0; i < rows; i++) {
    for (let j = 0; j < cols; j++) {
      const shape = Module._get_piece_shape(g, i, j);
      if (shape === 0) continue; // EMPTY

      for (let d = 0; d < 4; d++) {
        const status = Module._check_edge(g, i, j, d);
        if (status === 1) { // MISMATCH
          const x = offsetX + j * cellSize;
          const y = offsetY + i * cellSize;

          ctx.beginPath();
          switch (d) {
            case 0: // NORTH
              ctx.moveTo(x, y);
              ctx.lineTo(x + cellSize, y);
              break;
            case 1: // EAST
              ctx.moveTo(x + cellSize, y);
              ctx.lineTo(x + cellSize, y + cellSize);
              break;
            case 2: // SOUTH
              ctx.moveTo(x, y + cellSize);
              ctx.lineTo(x + cellSize, y + cellSize);
              break;
            case 3: // WEST
              ctx.moveTo(x, y);
              ctx.lineTo(x, y + cellSize);
              break;
          }
          ctx.stroke();
        }
      }
    }
  }
  ctx.restore();
}

// le mode Wrapping ON et OFF
function onToggleWrapping() {
  wrappingEnabled = !wrappingEnabled;
  const label = wrappingEnabled ? "Wrapping: ON" : "Wrapping: OFF";
  document.getElementById("wrap-button").innerText = label;
  drawGrid(game);
}

// le menu déroulant Type (des grilles wrapping et non wrapping sélectionnés)
function onSelectGrid() {
  const val = document.getElementById("type-select").value;

  if (val === "custom") {
    toggleCustomBox(); 
    return;
  }
  let rows = 5, cols = 5, wrapping = false;
  if (val.endsWith("w")) wrapping = true;

  const dims = val.replace("w", "").split("x").map(Number);
  rows = dims[0];
  cols = dims[1];

  if (game) Module._delete(game);
  game = Module._new_random(rows, cols, wrapping ? 1 : 0, 0, 0);
  Module._restart(game);
  drawGrid(game);

  wrappingEnabled = wrapping;
  document.getElementById("wrap-button").innerText = `Wrapping: ${wrapping ? "ON" : "OFF"}`;

  document.getElementById("type-select").selectedIndex = 0;
}

// un box caché pour la selection costume dans menu déroulant
function toggleCustomBox() {
  const box = document.getElementById("custom-box");
  box.classList.toggle("hidden");
}

// crée une grille avec les dimensions personnalisées (limit de taille 2 à 20)
function createCustomGrid() {
  const rows = parseInt(document.getElementById("custom-rows").value);
  const cols = parseInt(document.getElementById("custom-cols").value);

  if (!rows || !cols || rows < 2 || cols < 2|| rows > 20 || cols > 20) {
    alert("Please enter valid dimensions (min 2, max 20)");
    return;
  }

  const wrap = wrappingEnabled ? 1 : 0;
  if (game) Module._delete(game);
  game = Module._new_random(rows, cols, wrap, 0, 0);
  Module._set_wrapping(game, wrap);  
  Module._restart(game);
  drawGrid(game);

  toggleCustomBox();
  document.getElementById("type-select").selectedIndex = 0;
}

// taper "enter pour créer un grille perso"
document.querySelectorAll("#custom-rows, #custom-cols").forEach(input => {
  input.addEventListener("keydown", function (e) {
    if (e.key === "Enter") {
      createCustomGrid();
    }
  });
});

// Redimensionne le canvas à l’écran
function resizeCanvas() {
  const size = Math.min(window.innerWidth, window.innerHeight);
  canvas.width = size;
  canvas.height = size;

  if (typeof drawGrid === "function" && typeof game !== "undefined" && game) {
    drawGrid(game);
  }
}

window.addEventListener("resize", resizeCanvas);
window.addEventListener("load", resizeCanvas);