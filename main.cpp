#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdio>


const int WIN_W = 1000;
const int WIN_H = 700;

const int HUD_H      = 60;
const int BOARD_MARGIN = 20;


const int ARENA_X1 = BOARD_MARGIN;
const int ARENA_Y1 = BOARD_MARGIN;
const int ARENA_X2 = WIN_W - BOARD_MARGIN;
const int ARENA_Y2 = WIN_H - HUD_H - BOARD_MARGIN;

const int CELL = 20;


const int GRID_X1 = (ARENA_X1 / CELL) + 1;
const int GRID_Y1 = (ARENA_Y1 / CELL) + 1;
const int GRID_X2 = (ARENA_X2 / CELL) - 1;
const int GRID_Y2 = (ARENA_Y2 / CELL) - 1;

const int MAX_SNAKE = 2000;
const int INIT_LEN  = 5;

const int BASE_SPEED   = 150;
const int MIN_SPEED    = 55;
const int SCORE_PER_FOOD   = 10;
const int SCORE_PER_LEVEL  = 50;


enum GameState { START_SCREEN, PLAYING, PAUSED, GAME_OVER };

struct Seg { int x, y; };

GameState  gState      = START_SCREEN;
Seg        snake[MAX_SNAKE];
int        snakeLen    = 0;
int        dirX        = 1, dirY = 0;
int        nextDirX    = 1, nextDirY = 0;
int        foodX, foodY;
int        score       = 0;
int        highScore   = 0;
int        level       = 1;
int        moveInterval = BASE_SPEED;


bool       popupActive = false;
float      popupX, popupY;
int        popupTimer  = 0;


float      foodPulse   = 0.0f;
float      foodPulseDir= 1.0f;

struct Star { float x, y, size; };
const int NUM_STARS = 60;
Star stars[NUM_STARS];


void drawCircle(float cx, float cy, float r, int segs = 32)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segs; ++i)
    {
        float a = (float)i / segs * 2.0f * 3.14159265f;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}

void drawCircleOutline(float cx, float cy, float r, int segs = 32)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segs; ++i)
    {
        float a = (float)i / segs * 2.0f * 3.14159265f;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}


void drawRoundRect(float x, float y, float w, float h, float r, int segs = 10)
{
    float cx[4] = { x+r,     x+w-r,   x+w-r,  x+r   };
    float cy[4] = { y+r,     y+r,     y+h-r,  y+h-r };
    float startA[4] = { 180.0f, 270.0f, 0.0f, 90.0f };

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + w*0.5f, y + h*0.5f);
    for (int c = 0; c < 4; ++c)
        for (int i = 0; i <= segs; ++i)
        {
            float a = (startA[c] + (float)i / segs * 90.0f) * 3.14159265f / 180.0f;
            glVertex2f(cx[c] + cosf(a)*r, cy[c] + sinf(a)*r);
        }
    glEnd();
}


void drawText(float x, float y, const char* str, void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    while (*str) { glutBitmapCharacter(font, *str++); }
}

void drawTextLarge(float x, float y, const char* str)
{
    drawText(x, y, str, GLUT_BITMAP_HELVETICA_18);
}

void drawTextHuge(float x, float y, const char* str)
{
    glRasterPos2f(x, y);
    while (*str) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str++); }
}


int strWidth(const char* s, void* font = GLUT_BITMAP_HELVETICA_18)
{
    int w = 0;
    while (*s) { w += glutBitmapWidth(font, *s++); }
    return w;
}


void spawnFood()
{
    int attempts = 0;
    while (attempts < 10000)
    {
        ++attempts;
        int fx = GRID_X1 + rand() % (GRID_X2 - GRID_X1);
        int fy = GRID_Y1 + rand() % (GRID_Y2 - GRID_Y1);
        bool onSnake = false;
        for (int i = 0; i < snakeLen; ++i)
            if (snake[i].x == fx && snake[i].y == fy) { onSnake = true; break; }
        if (!onSnake) { foodX = fx; foodY = fy; return; }
    }
    // fallback — shouldn't happen unless board is nearly full
    foodX = GRID_X1; foodY = GRID_Y1;
}

// ============================================================
//  INITIALIZE / RESTART
// ============================================================
void initGame()
{
    snakeLen = INIT_LEN;
    int startX = (GRID_X1 + GRID_X2) / 2;
    int startY = (GRID_Y1 + GRID_Y2) / 2;
    for (int i = 0; i < snakeLen; ++i)
    {
        snake[i].x = startX - i;
        snake[i].y = startY;
    }
    dirX = 1; dirY = 0;
    nextDirX = 1; nextDirY = 0;
    score = 0;
    level = 1;
    moveInterval = BASE_SPEED;
    popupActive = false;
    spawnFood();
}

// ============================================================
//  SPEED / LEVEL
// ============================================================
void updateDifficulty()
{
    level = 1 + score / SCORE_PER_LEVEL;
    if (level > 10) level = 10;
    int speed = BASE_SPEED - (level - 1) * 10;
    if (speed < MIN_SPEED) speed = MIN_SPEED;
    moveInterval = speed;
}

// ============================================================
//  DRAWING HELPERS — grid cell centre in world coords
// ============================================================
float cellCX(int gx) { return gx * CELL + CELL * 0.5f; }
float cellCY(int gy) { return gy * CELL + CELL * 0.5f; }

// ============================================================
//  DRAW BACKGROUND (window bg + stars)
// ============================================================
void drawBackground()
{
    // deep space background
    glColor3f(0.04f, 0.04f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIN_W, 0);
    glVertex2f(WIN_W, WIN_H);
    glVertex2f(0, WIN_H);
    glEnd();

    // stars
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; ++i)
        glVertex2f(stars[i].x, stars[i].y);
    glEnd();
    glPointSize(1.0f);
}

// ============================================================
//  DRAW HUD
// ============================================================
void drawHUD()
{
    // HUD background bar
    glColor3f(0.08f, 0.08f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0, WIN_H - HUD_H);
    glVertex2f(WIN_W, WIN_H - HUD_H);
    glVertex2f(WIN_W, WIN_H);
    glVertex2f(0, WIN_H);
    glEnd();

    // Separator line
    glColor3f(0.20f, 0.80f, 0.40f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(0, WIN_H - HUD_H);
    glVertex2f(WIN_W, WIN_H - HUD_H);
    glEnd();
    glLineWidth(1.0f);

    // Labels
    char buf[128];

    // SCORE
    glColor3f(0.20f, 0.90f, 0.45f);
    sprintf(buf, "SCORE: %d", score);
    drawTextHuge(30, WIN_H - 35, buf);

    // HIGH SCORE
    glColor3f(1.0f, 0.85f, 0.20f);
    sprintf(buf, "BEST: %d", highScore);
    drawTextHuge(280, WIN_H - 35, buf);

    // LENGTH
    glColor3f(0.50f, 0.75f, 1.00f);
    sprintf(buf, "LEN: %d", snakeLen);
    drawTextHuge(520, WIN_H - 35, buf);

    // LEVEL
    glColor3f(1.0f, 0.50f, 0.20f);
    sprintf(buf, "LVL: %d", level);
    drawTextHuge(700, WIN_H - 35, buf);

    // Controls hint
    glColor3f(0.40f, 0.40f, 0.55f);
    drawText(820, WIN_H - 40, "P=Pause R=Restart ESC=Exit", GLUT_BITMAP_HELVETICA_12);
}

// ============================================================
//  DRAW ARENA BOARD
// ============================================================
void drawGameBoard()
{
    // Outer glow border
    glColor3f(0.20f, 0.70f, 0.35f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(ARENA_X1, ARENA_Y1);
    glVertex2f(ARENA_X2, ARENA_Y1);
    glVertex2f(ARENA_X2, ARENA_Y2);
    glVertex2f(ARENA_X1, ARENA_Y2);
    glEnd();
    glLineWidth(1.0f);

    // Inner arena fill
    glColor3f(0.05f, 0.07f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(ARENA_X1+2, ARENA_Y1+2);
    glVertex2f(ARENA_X2-2, ARENA_Y1+2);
    glVertex2f(ARENA_X2-2, ARENA_Y2-2);
    glVertex2f(ARENA_X1+2, ARENA_Y2-2);
    glEnd();
}

// ============================================================
//  DRAW GRID
// ============================================================
void drawGrid()
{
    glColor4f(0.12f, 0.15f, 0.22f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    // vertical
    for (int gx = GRID_X1; gx <= GRID_X2; ++gx)
    {
        float wx = gx * CELL;
        glVertex2f(wx, ARENA_Y1+2);
        glVertex2f(wx, ARENA_Y2-2);
    }
    // horizontal
    for (int gy = GRID_Y1; gy <= GRID_Y2; ++gy)
    {
        float wy = gy * CELL;
        glVertex2f(ARENA_X1+2, wy);
        glVertex2f(ARENA_X2-2, wy);
    }
    glEnd();
}

// ============================================================
//  DRAW FOOD (apple-like)
// ============================================================
void drawFood()
{
    float cx = cellCX(foodX);
    float cy = cellCY(foodY);
    float r  = (CELL * 0.38f) + foodPulse * 2.5f;

    // shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
    drawCircle(cx + 2, cy - 2, r);

    // main red body
    glColor3f(0.92f, 0.15f, 0.15f);
    drawCircle(cx, cy, r);

    // specular highlight
    glColor3f(1.0f, 0.55f, 0.55f);
    drawCircle(cx - r*0.28f, cy + r*0.28f, r*0.35f);

    // tiny white glint
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(cx - r*0.28f, cy + r*0.30f, r*0.12f);

    // stem
    glColor3f(0.45f, 0.25f, 0.05f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(cx, cy + r);
    glVertex2f(cx + 3, cy + r + 6);
    glEnd();
    glLineWidth(1.0f);

    // leaf
    glColor3f(0.15f, 0.75f, 0.20f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx + 3, cy + r + 6);
    glVertex2f(cx + 3, cy + r + 3);
    glVertex2f(cx + 8, cy + r + 7);
    glVertex2f(cx + 5, cy + r + 10);
    glEnd();

    // outer outline
    glColor3f(0.60f, 0.05f, 0.05f);
    glLineWidth(1.5f);
    drawCircleOutline(cx, cy, r);
    glLineWidth(1.0f);
}

// ============================================================
//  DRAW SNAKE BODY SEGMENT
// ============================================================
void drawBodySegment(float cx, float cy, float r, int idx)
{
    // colour gradient — head is bright green, tail fades
    float t = 1.0f - (float)idx / snakeLen;   // 1 near head, 0 near tail
    float gr = 0.10f + t * 0.75f;
    float gg = 0.60f + t * 0.35f;
    float gb = 0.10f + t * 0.15f;

    // shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.28f);
    drawCircle(cx + 1.5f, cy - 1.5f, r * 0.92f);

    // main body
    glColor3f(gr, gg, gb);
    drawCircle(cx, cy, r * 0.92f);

    // inner highlight
    glColor3f(gr + 0.15f, gg + 0.10f, gb + 0.10f);
    drawCircle(cx - r*0.18f, cy + r*0.18f, r * 0.40f);

    // dark outline
    glColor3f(gr * 0.35f, gg * 0.35f, gb * 0.35f);
    glLineWidth(1.2f);
    drawCircleOutline(cx, cy, r * 0.92f);
    glLineWidth(1.0f);

    // scale dot
    if (idx % 2 == 0)
    {
        glColor3f(gr * 0.50f, gg * 0.50f, gb * 0.50f);
        drawCircle(cx, cy, r * 0.22f);
    }
}

// ============================================================
//  DRAW SNAKE HEAD
// ============================================================
void drawSnakeHead()
{
    if (snakeLen == 0) return;
    float cx = cellCX(snake[0].x);
    float cy = cellCY(snake[0].y);
    float r  = CELL * 0.48f;

    // rotation angle based on direction
    float angle = 0.0f;
    if      (dirX ==  1) angle =   0.0f;
    else if (dirX == -1) angle = 180.0f;
    else if (dirY ==  1) angle =  90.0f;
    else if (dirY == -1) angle = 270.0f;

    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    // shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
    drawCircle(2.0f, -2.0f, r);

    // head body (bright green)
    glColor3f(0.15f, 0.92f, 0.30f);
    drawCircle(0.0f, 0.0f, r);

    // forehead highlight
    glColor3f(0.40f, 1.00f, 0.55f);
    drawCircle(-r*0.22f, r*0.22f, r * 0.42f);

    // snout ellipse (slightly elongated in +X direction)
    glColor3f(0.10f, 0.72f, 0.22f);
    glPushMatrix();
    glTranslatef(r * 0.35f, 0.0f, 0.0f);
    glScalef(1.2f, 0.85f, 1.0f);
    drawCircle(0.0f, 0.0f, r * 0.48f);
    glPopMatrix();

    // LEFT eye white
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(r*0.15f,  r*0.42f, r*0.24f);
    // LEFT pupil
    glColor3f(0.05f, 0.05f, 0.05f);
    drawCircle(r*0.20f,  r*0.42f, r*0.13f);
    // LEFT shine
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(r*0.23f,  r*0.45f, r*0.05f);

    // RIGHT eye white
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(r*0.15f, -r*0.42f, r*0.24f);
    // RIGHT pupil
    glColor3f(0.05f, 0.05f, 0.05f);
    drawCircle(r*0.20f, -r*0.42f, r*0.13f);
    // RIGHT shine
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(r*0.23f, -r*0.45f, r*0.05f);

    // Nostril dots
    glColor3f(0.06f, 0.50f, 0.10f);
    drawCircle(r*0.62f,  r*0.14f, r*0.07f);
    drawCircle(r*0.62f, -r*0.14f, r*0.07f);

    // Tongue (forked, red)
    glColor3f(0.90f, 0.08f, 0.08f);
    glLineWidth(2.5f);
    // tongue base
    glBegin(GL_LINES);
    glVertex2f(r*0.80f,  0.0f);
    glVertex2f(r*1.20f,  0.0f);
    glEnd();
    // fork
    glBegin(GL_LINES);
    glVertex2f(r*1.20f,  0.0f);
    glVertex2f(r*1.45f,  r*0.18f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(r*1.20f,  0.0f);
    glVertex2f(r*1.45f, -r*0.18f);
    glEnd();
    glLineWidth(1.0f);

    // outline
    glColor3f(0.05f, 0.40f, 0.10f);
    glLineWidth(1.5f);
    drawCircleOutline(0.0f, 0.0f, r);
    glLineWidth(1.0f);

    glPopMatrix();
}

// ============================================================
//  DRAW FULL SNAKE
// ============================================================
void drawSnake()
{
    float r = CELL * 0.46f;

    // body segments (skip index 0 = head)
    for (int i = snakeLen - 1; i >= 1; --i)
        drawBodySegment(cellCX(snake[i].x), cellCY(snake[i].y), r, i);

    // head on top
    drawSnakeHead();
}

// ============================================================
//  DRAW SCORE POP-UP
// ============================================================
void drawPopup()
{
    if (!popupActive) return;
    float alpha = (float)popupTimer / 30.0f;
    glColor4f(1.0f, 0.90f, 0.10f, alpha);
    char buf[] = "+10";
    drawTextHuge(popupX - 12, popupY, buf);
}

// ============================================================
//  DRAW START SCREEN
// ============================================================
void drawStartScreen()
{
    drawBackground();

    // Panel
    glColor4f(0.05f, 0.08f, 0.16f, 0.95f);
    drawRoundRect(200, 130, 600, 440, 18);

    // Panel border
    glColor3f(0.20f, 0.80f, 0.40f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(200, 130);
    glVertex2f(800, 130);
    glVertex2f(800, 570);
    glVertex2f(200, 570);
    glEnd();
    glLineWidth(1.0f);

    // Title
    glColor3f(0.20f, 0.95f, 0.45f);
    const char* title = "SNAKE GAME";
    float tw = strWidth(title, GLUT_BITMAP_TIMES_ROMAN_24);
    drawTextHuge(500 - tw * 0.5f, 515, title);

    // Subtitle
    glColor3f(1.0f, 0.85f, 0.20f);
    const char* sub = "CLASSIC ARCADE ADVENTURE";
    float sw2 = strWidth(sub, GLUT_BITMAP_HELVETICA_12);
    drawText(500 - sw2 * 0.5f, 488, sub, GLUT_BITMAP_HELVETICA_12);

    // Decorative line
    glColor3f(0.20f, 0.80f, 0.40f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(230, 470); glVertex2f(770, 470);
    glEnd();
    glLineWidth(1.0f);

    // Controls heading
    glColor3f(0.70f, 0.85f, 1.0f);
    const char* ch = "CONTROLS";
    float chw = strWidth(ch, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - chw * 0.5f, 448, ch);

    // Controls list
    struct { const char* k; const char* d; } controls[] = {
        { "UP ARROW",    "Move Up"        },
        { "DOWN ARROW",  "Move Down"      },
        { "LEFT ARROW",  "Move Left"      },
        { "RIGHT ARROW", "Move Right"     },
        { "P",           "Pause / Resume" },
        { "R",           "Restart"        },
        { "ESC",         "Exit"           },
    };
    int nc = 7;
    float cy2 = 416;
    for (int i = 0; i < nc; ++i, cy2 -= 28)
    {
        glColor3f(1.0f, 0.75f, 0.20f);
        drawText(290, cy2, controls[i].k, GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.80f, 0.80f, 0.80f);
        drawText(440, cy2, controls[i].d, GLUT_BITMAP_HELVETICA_12);
    }

    // Decorative line
    glColor3f(0.20f, 0.80f, 0.40f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(230, 225); glVertex2f(770, 225);
    glEnd();
    glLineWidth(1.0f);

    // Press ENTER
    glColor3f(0.20f, 0.95f, 0.45f);
    const char* enter = "Press  ENTER  to Start";
    float ew = strWidth(enter, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - ew * 0.5f, 183, enter);

    // High score if any
    if (highScore > 0)
    {
        char hbuf[64];
        sprintf(hbuf, "Best Score: %d", highScore);
        glColor3f(1.0f, 0.85f, 0.20f);
        float hw = strWidth(hbuf, GLUT_BITMAP_HELVETICA_12);
        drawText(500 - hw * 0.5f, 155, hbuf, GLUT_BITMAP_HELVETICA_12);
    }
}

// ============================================================
//  DRAW PAUSE SCREEN
// ============================================================
void drawPauseScreen()
{
    // translucent overlay
    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);  glVertex2f(WIN_W, 0);
    glVertex2f(WIN_W, WIN_H); glVertex2f(0, WIN_H);
    glEnd();

    // Panel
    glColor4f(0.05f, 0.08f, 0.18f, 0.95f);
    drawRoundRect(300, 270, 400, 160, 14);

    glColor3f(0.20f, 0.80f, 0.40f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(300, 270); glVertex2f(700, 270);
    glVertex2f(700, 430); glVertex2f(300, 430);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(1.0f, 0.90f, 0.20f);
    const char* p = "GAME  PAUSED";
    float pw = strWidth(p, GLUT_BITMAP_TIMES_ROMAN_24);
    drawTextHuge(500 - pw * 0.5f, 392, p);

    glColor3f(0.80f, 0.80f, 0.80f);
    const char* r = "Press P to Resume";
    float rw = strWidth(r, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - rw * 0.5f, 308, r);
}

// ============================================================
//  DRAW GAME OVER SCREEN
// ============================================================
void drawGameOverScreen()
{
    // overlay
    glColor4f(0.0f, 0.0f, 0.0f, 0.72f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);  glVertex2f(WIN_W, 0);
    glVertex2f(WIN_W, WIN_H); glVertex2f(0, WIN_H);
    glEnd();

    // Panel
    glColor4f(0.08f, 0.04f, 0.12f, 0.97f);
    drawRoundRect(250, 180, 500, 340, 16);

    glColor3f(0.90f, 0.15f, 0.15f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(250, 180); glVertex2f(750, 180);
    glVertex2f(750, 520); glVertex2f(250, 520);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.95f, 0.15f, 0.15f);
    const char* go = "GAME  OVER";
    float gw = strWidth(go, GLUT_BITMAP_TIMES_ROMAN_24);
    drawTextHuge(500 - gw * 0.5f, 482, go);

    // Stats
    char buf[64];
    float sy = 430;

    glColor3f(0.20f, 0.90f, 0.45f);
    sprintf(buf, "Score :  %d", score);
    float bw = strWidth(buf, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - bw * 0.5f, sy, buf); sy -= 38;

    glColor3f(1.0f, 0.85f, 0.20f);
    sprintf(buf, "Best  :  %d", highScore);
    bw = strWidth(buf, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - bw * 0.5f, sy, buf); sy -= 38;

    glColor3f(0.50f, 0.75f, 1.00f);
    sprintf(buf, "Length:  %d", snakeLen);
    bw = strWidth(buf, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - bw * 0.5f, sy, buf); sy -= 38;

    glColor3f(1.0f, 0.55f, 0.20f);
    sprintf(buf, "Level :  %d", level);
    bw = strWidth(buf, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - bw * 0.5f, sy, buf); sy -= 50;

    // Divider
    glColor3f(0.45f, 0.10f, 0.10f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(280, sy + 18); glVertex2f(720, sy + 18);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.90f, 0.90f, 0.90f);
    const char* r2 = "Press  R  to Restart";
    float r2w = strWidth(r2, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - r2w * 0.5f, sy - 6, r2); sy -= 30;

    glColor3f(0.65f, 0.65f, 0.65f);
    const char* e2 = "Press  ESC  to Exit";
    float e2w = strWidth(e2, GLUT_BITMAP_HELVETICA_18);
    drawText(500 - e2w * 0.5f, sy - 6, e2);
}

// ============================================================
//  MAIN DISPLAY CALLBACK
// ============================================================
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (gState == START_SCREEN)
    {
        drawStartScreen();
    }
    else if (gState == PLAYING || gState == PAUSED)
    {
        drawBackground();
        drawGameBoard();
        drawGrid();
        drawFood();
        drawSnake();
        drawPopup();
        drawHUD();
        if (gState == PAUSED) drawPauseScreen();
    }
    else if (gState == GAME_OVER)
    {
        drawBackground();
        drawGameBoard();
        drawGrid();
        drawFood();
        drawSnake();
        drawHUD();
        drawGameOverScreen();
    }

    glutSwapBuffers();
}

// ============================================================
//  MOVE SNAKE
// ============================================================
void moveSnake()
{
    // commit buffered direction
    dirX = nextDirX;
    dirY = nextDirY;

    // new head position
    int nx = snake[0].x + dirX;
    int ny = snake[0].y + dirY;

    // wall collision
    if (nx < GRID_X1 || nx >= GRID_X2 || ny < GRID_Y1 || ny >= GRID_Y2)
    {
        if (score > highScore) highScore = score;
        gState = GAME_OVER;
        return;
    }

    // self collision
    for (int i = 1; i < snakeLen; ++i)
        if (snake[i].x == nx && snake[i].y == ny)
        {
            if (score > highScore) highScore = score;
            gState = GAME_OVER;
            return;
        }

    // food collision
    bool ate = (nx == foodX && ny == foodY);

    // shift body
    if (ate && snakeLen < MAX_SNAKE)
    {
        // grow: don't remove tail
        for (int i = snakeLen; i > 0; --i)
            snake[i] = snake[i-1];
        snakeLen++;
    }
    else
    {
        for (int i = snakeLen - 1; i > 0; --i)
            snake[i] = snake[i-1];
    }
    snake[0].x = nx;
    snake[0].y = ny;

    if (ate)
    {
        score += SCORE_PER_FOOD;
        if (score > highScore) highScore = score;
        updateDifficulty();
        // pop-up
        popupX = cellCX(nx);
        popupY = cellCY(ny) + 12;
        popupTimer = 30;
        popupActive = true;
        spawnFood();
    }
}

// ============================================================
//  TIMER CALLBACK
// ============================================================
void timerMove(int)
{
    if (gState == PLAYING)
    {
        moveSnake();
        // food pulse
        foodPulse += foodPulseDir * 0.07f;
        if (foodPulse >  1.0f) { foodPulse =  1.0f; foodPulseDir = -1.0f; }
        if (foodPulse < -0.5f) { foodPulse = -0.5f; foodPulseDir =  1.0f; }
        // pop-up countdown
        if (popupActive && --popupTimer <= 0) popupActive = false;
    }
    glutPostRedisplay();
    glutTimerFunc(moveInterval, timerMove, 0);
}

// ============================================================
//  VISUAL TIMER (for food pulse even when paused)
// ============================================================
void timerVisual(int)
{
    if (gState != PLAYING)  // pulse still ticks for aesthetics on pause/go screens
    {
        foodPulse += foodPulseDir * 0.07f;
        if (foodPulse >  1.0f) { foodPulse =  1.0f; foodPulseDir = -1.0f; }
        if (foodPulse < -0.5f) { foodPulse = -0.5f; foodPulseDir =  1.0f; }
        glutPostRedisplay();
    }
    glutTimerFunc(60, timerVisual, 0);
}

// ============================================================
//  KEYBOARD (regular keys)
// ============================================================
void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
        case 27: // ESC
            exit(0);

        case 13: // ENTER
            if (gState == START_SCREEN)
            {
                initGame();
                gState = PLAYING;
            }
            break;

        case 'p': case 'P':
            if (gState == PLAYING)  gState = PAUSED;
            else if (gState == PAUSED) gState = PLAYING;
            break;

        case 'r': case 'R':
            if (gState == GAME_OVER || gState == PLAYING || gState == PAUSED)
            {
                initGame();
                gState = PLAYING;
            }
            break;
    }
    glutPostRedisplay();
}

// ============================================================
//  SPECIAL KEYS (arrows)
// ============================================================
void specialKeyboard(int key, int /*x*/, int /*y*/)
{
    if (gState != PLAYING) return;
    switch (key)
    {
        case GLUT_KEY_UP:
            if (dirY != -1) { nextDirX = 0; nextDirY = 1; }
            break;
        case GLUT_KEY_DOWN:
            if (dirY !=  1) { nextDirX = 0; nextDirY = -1; }
            break;
        case GLUT_KEY_LEFT:
            if (dirX !=  1) { nextDirX = -1; nextDirY = 0; }
            break;
        case GLUT_KEY_RIGHT:
            if (dirX != -1) { nextDirX = 1; nextDirY = 0; }
            break;
    }
}

// ============================================================
//  RESHAPE
// ============================================================
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ============================================================
//  INITIALIZATION
// ============================================================
void setupGL()
{
    glClearColor(0.04f, 0.04f, 0.10f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

void initStars()
{
    for (int i = 0; i < NUM_STARS; ++i)
    {
        stars[i].x    = (float)(rand() % WIN_W);
        stars[i].y    = (float)(rand() % (WIN_H - HUD_H));
        stars[i].size = 1.0f + (float)(rand() % 3);
    }
}

// ============================================================
//  MAIN
// ============================================================
int main(int argc, char** argv)
{
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Snake Game -- Graphics Lab Project");

    setupGL();
    initStars();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);

    glutTimerFunc(BASE_SPEED, timerMove, 0);
    glutTimerFunc(60,         timerVisual, 0);

    glutMainLoop();
    return 0;
}
