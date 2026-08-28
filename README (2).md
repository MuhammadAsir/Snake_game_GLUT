# 🐍 Snake Game — 2D Animated Arcade Game

> A fully polished, single-file 2D Snake Game built with **C++**, **OpenGL**, and **GLUT** as a university Computer Graphics Lab project. Everything — graphics, animation, logic, and UI — is rendered entirely through OpenGL primitives. No external assets. No textures. No additional files.

---

## 📸 Overview

This project is a modernized take on the classic Snake arcade game, demonstrating core computer graphics concepts including 2D transformations, animation, collision detection, keyboard interaction, and dynamic object movement — all within a single `main.cpp` file.

### Screenshots

| Early Game | Mid Game |
|:---:|:---:|
| ![Snake game at score 0, length 5, level 1](screenshot1.png) | ![Snake game at score 80, length 13, level 2, with score pop-up](screenshot2.png) |
| Score: 0 · Length: 5 · Level: 1 | Score: 80 · Length: 13 · Level: 2 |

---

## 🎮 What the Game Does

- A snake moves continuously across a bounded arena
- The player steers the snake using arrow keys
- Eating food grows the snake and increases the score
- The game ends if the snake hits a wall or its own body
- Difficulty increases automatically as the score rises
- A HUD displays live score, best score, length, and level

---

## 🗂️ Game States

| State | Description |
|---|---|
| `START_SCREEN` | Title screen shown on launch |
| `PLAYING` | Active gameplay |
| `PAUSED` | Game frozen, overlay displayed |
| `GAME_OVER` | Death screen with final stats |

---

## 🕹️ Controls

| Key | Action |
|---|---|
| `↑ ↓ ← →` | Move snake |
| `Enter` | Start game from title screen |
| `P` | Pause / Resume |
| `R` | Restart (from any state) |
| `ESC` | Exit |

---

## 🧩 Features

- **Animated snake head** — eyes, pupils, tongue, and snout rotate to face the movement direction using `glRotatef()`
- **Gradient body** — each body segment shifts from bright green near the head to dark green near the tail
- **Apple food** — drawn with circular body, specular highlight, glint, stem, and leaf using only OpenGL primitives
- **Food pulse animation** — the apple gently pulses in size every frame
- **Score pop-up** — a `+10` label briefly appears at the position food was eaten
- **Grid overlay** — subtle grid lines drawn across the arena
- **Star field background** — randomized decorative stars behind the arena
- **HUD bar** — always-visible top bar showing score, best, length, and level
- **Progressive difficulty** — movement speed increases every 50 points, across 10 levels
- **High score** — persists across restarts within the same session
- **Illegal move prevention** — the snake cannot instantly reverse direction

---

## ⚙️ How It Works

### Coordinate System
The window is `1000 × 700` pixels using a 2D orthographic projection:
```
gluOrtho2D(0, 1000, 0, 700);
```
The arena sits inside this window with a top HUD strip and margins.

### Grid-Based Movement
The snake lives on a logical grid of `CELL_SIZE = 20` pixels per cell. All positions — snake segments and food — are stored as grid coordinates `(x, y)` and converted to world coordinates for rendering:
```cpp
float cellCX(int gx) { return gx * CELL + CELL * 0.5f; }
float cellCY(int gy) { return gy * CELL + CELL * 0.5f; }
```

### Snake Storage
```cpp
struct Seg { int x, y; };
Seg snake[MAX_SNAKE];  // MAX_SNAKE = 2000
int snakeLen;
```
On each move, the body shifts forward. When food is eaten, the tail is kept rather than removed, growing the snake by one cell.

### Timer-Driven Animation
GLUT timer callbacks drive both game logic and visuals:
```cpp
glutTimerFunc(moveInterval, timerMove, 0);   // snake movement
glutTimerFunc(60, timerVisual, 0);           // food pulse & redraws
```
`moveInterval` decreases as level increases, making the snake faster.

---

## 🔧 Key Functions

### Drawing
| Function | Purpose |
|---|---|
| `drawBackground()` | Renders the dark space background and star field |
| `drawGameBoard()` | Draws the arena border and inner fill |
| `drawGrid()` | Draws subtle grid lines inside the arena |
| `drawSnake()` | Renders all body segments then the head on top |
| `drawSnakeHead()` | Draws the detailed animated head using transformations |
| `drawBodySegment()` | Draws one gradient body circle with highlight and outline |
| `drawFood()` | Draws the pulsing apple with stem, leaf, and glint |
| `drawHUD()` | Renders the top info bar |
| `drawStartScreen()` | Title screen with controls list |
| `drawPauseScreen()` | Semi-transparent pause overlay |
| `drawGameOverScreen()` | Game over overlay with final stats |
| `drawPopup()` | Fading `+10` score pop-up near eaten food |
| `drawCircle()` | Utility — filled circle via `GL_TRIANGLE_FAN` |
| `drawCircleOutline()` | Utility — circle outline via `GL_LINE_LOOP` |
| `drawRoundRect()` | Utility — filled rounded rectangle |
| `drawText()` | Utility — renders a GLUT bitmap string |

### Logic
| Function | Purpose |
|---|---|
| `initGame()` | Resets all game state for a new game |
| `moveSnake()` | Advances the snake one cell, checks all collisions |
| `spawnFood()` | Places food at a random valid grid cell |
| `updateDifficulty()` | Recalculates level and move speed from score |
| `checkWallCollision()` | Handled inside `moveSnake()` — head vs. arena bounds |
| `checkSelfCollision()` | Handled inside `moveSnake()` — head vs. body segments |
| `checkFoodCollision()` | Handled inside `moveSnake()` — head vs. food cell |

### Callbacks
| Function | Purpose |
|---|---|
| `display()` | Main GLUT display callback — routes drawing by game state |
| `keyboard()` | Handles `Enter`, `P`, `R`, `ESC` |
| `specialKeyboard()` | Handles arrow key direction input |
| `timerMove()` | Called every `moveInterval` ms — moves snake, ticks food pulse |
| `timerVisual()` | Called every 60 ms — keeps visuals alive while paused |
| `reshape()` | Maintains correct projection on window resize |

### Transformations Used
```cpp
glPushMatrix();
    glTranslatef(cx, cy, 0.0f);   // move to head position
    glRotatef(angle, 0,0,1);      // rotate head to face direction
    glScalef(1.2f, 0.85f, 1.0f); // scale snout ellipse
    // ... draw head parts ...
glPopMatrix();
```

---

## 📐 Graphics Concepts Demonstrated

| Concept | Where |
|---|---|
| 2D Orthographic projection | `gluOrtho2D` |
| Translation | Snake head, food, segments |
| Rotation | Snake head facing direction |
| Scaling | Snout shape, food pulse |
| `glPushMatrix / glPopMatrix` | Head rendering |
| Filled primitives | `GL_TRIANGLE_FAN`, `GL_QUADS` |
| Line primitives | `GL_LINES`, `GL_LINE_LOOP` |
| Alpha blending | Overlays, shadows, pop-ups |
| Timer animation | `glutTimerFunc` |
| Keyboard interaction | `glutKeyboardFunc`, `glutSpecialFunc` |
| Collision detection | Wall, self, food |
| Randomization | `srand / rand` for food placement |
| Game state machine | `START → PLAYING → PAUSED / GAME_OVER` |

---

## 📋 Requirements to Run

1. **Code::Blocks** — IDE used to compile and run the project
2. **GLUT** (configured inside Code::Blocks) — OpenGL Utility Toolkit for windowing and input

> No additional libraries, assets, images, or files are needed. The entire program is self-contained in `main.cpp`.

---

## 🛠️ Setup Instructions

Code::Blocks and GLUT/freeGLUT setup can be installed in your system using the following video as a reference: [https://youtu.be/7rLo69vCooU?si=ZO7bpbC04ztLNO1i](https://youtu.be/7rLo69vCooU?si=ZO7bpbC04ztLNO1i)

Once you have Code::Blocks with a GLUT setup ready, follow these steps:

1. Open Code::Blocks.
2. Tap on **Create a new project**, then tap on **GLUT project**.
3. Name your project, and update the project location if necessary.
4. For the GLUT location, copy the path of the `x86_64-w64-mingw32` folder inside your Code::Blocks folder, or manually select that. After that, a new project will be created.
   > **Note:** Make sure the **Compiler's installation directory** field in **Global Compiler Settings → Toolchain Executables** is set to the path of the MinGW folder inside your Code::Blocks folder. To get there, follow: `Code::Blocks → Settings → Compiler → Toolchain Executables`.
5. Once your project has been created, tap on the project name — you'll see **Sources**. Tap on Sources and you'll find `main.cpp`. Click on it to see the existing code. Build and run it (or simply press **F9**) — if a window appears, that confirms your GLUT setup is working. Now cut all the code there, copy the code from the `main.cpp` file of this repository, and paste it into Code::Blocks. Then press **F9** and you'll be able to play the game.

---

## 👥 Project Info

- **Type:** University Computer Graphics Lab Group Project
- **Language:** C++
- **Graphics API:** OpenGL + GLUT
- **File count:** 1 (`main.cpp`)
- **External assets:** None

---
