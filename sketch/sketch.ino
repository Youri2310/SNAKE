#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define PIN_SDA 18
#define PIN_SCL 17

#define CELL 4
#define GRID_W (SCREEN_WIDTH / CELL)
#define GRID_H (SCREEN_HEIGHT / CELL)
#define MAX_LEN (GRID_W * GRID_H)
#define STEP_MS 120

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 100000, 100000);

struct Point { int8_t x; int8_t y; };

struct Snake {
  Point body[MAX_LEN];
  int len;
  int8_t dirX;
  int8_t dirY;
  int8_t nextX;
  int8_t nextY;
};

Snake p1;
Snake p2;
Point food;
bool walls;
bool twoPlayers;
bool inMenu;
int menuStage;
bool selMulti;
unsigned long lastStep;

bool onSnakeCell(Snake &s, int8_t x, int8_t y) {
  for (int i = 0; i < s.len; i++)
    if (s.body[i].x == x && s.body[i].y == y) return true;
  return false;
}

void placeFood() {
  do {
    food.x = random(GRID_W);
    food.y = random(GRID_H);
  } while (onSnakeCell(p1, food.x, food.y) || (twoPlayers && onSnakeCell(p2, food.x, food.y)));
}

void initSnake(Snake &s, int cx, int cy, int8_t facing) {
  s.len = 3;
  s.body[0] = { (int8_t)cx, (int8_t)cy };
  s.body[1] = { (int8_t)(cx - facing), (int8_t)cy };
  s.body[2] = { (int8_t)(cx - 2 * facing), (int8_t)cy };
  s.dirX = facing;
  s.dirY = 0;
  s.nextX = 0;
  s.nextY = 0;
}

void resetGame() {
  if (twoPlayers) {
    initSnake(p1, GRID_W / 4, GRID_H / 2, 1);
    initSnake(p2, 3 * GRID_W / 4, GRID_H / 2, -1);
    p1.nextX = p1.dirX;
    p1.nextY = p1.dirY;
    p2.nextX = p2.dirX;
    p2.nextY = p2.dirY;
  } else {
    initSnake(p1, GRID_W / 2, GRID_H / 2, 1);
  }
  placeFood();
}

void drawMenu(const char *title, const char *opt1, const char *opt2) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(6, 6);
  display.print(title);
  display.setCursor(6, 30);
  display.print("&: ");
  display.print(opt1);
  display.setCursor(6, 46);
  display.print("e: ");
  display.print(opt2);
  display.display();
}

void showMenu() {
  if (menuStage == 0) drawMenu("Joueurs", "Solo", "Multi");
  else drawMenu("Terrain", "Sans murs", "Avec murs");
}

void gameOver() {
  inMenu = true;
  menuStage = 0;
  showMenu();
}

void winScreen(const char *msg) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(30, 28);
  display.print(msg);
  display.display();
  delay(2000);
  gameOver();
}

void draw() {
  display.clearDisplay();
  if (walls)
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.fillRect(food.x * CELL + 1, food.y * CELL + 1, CELL - 2, CELL - 2, SSD1306_WHITE);
  for (int i = 0; i < p1.len; i++)
    display.fillRect(p1.body[i].x * CELL, p1.body[i].y * CELL, CELL, CELL, SSD1306_WHITE);
  if (twoPlayers)
    for (int i = 0; i < p2.len; i++)
      display.drawRect(p2.body[i].x * CELL, p2.body[i].y * CELL, CELL, CELL, SSD1306_WHITE);
  display.display();
}

Point advance(Snake &s, bool &wallDead) {
  int nx = s.body[0].x + s.dirX;
  int ny = s.body[0].y + s.dirY;
  wallDead = false;
  if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) {
    if (walls) {
      wallDead = true;
      return s.body[0];
    }
    if (nx < 0) nx = GRID_W - 1;
    else if (nx >= GRID_W) nx = 0;
    if (ny < 0) ny = GRID_H - 1;
    else if (ny >= GRID_H) ny = 0;
  }
  return { (int8_t)nx, (int8_t)ny };
}

bool hits(Snake &s, Point p, int skip) {
  for (int i = 0; i < s.len - skip; i++)
    if (s.body[i].x == p.x && s.body[i].y == p.y) return true;
  return false;
}

void grow(Snake &s, Point head, bool eat) {
  int last = s.len < MAX_LEN ? s.len : MAX_LEN - 1;
  for (int i = last; i > 0; i--) s.body[i] = s.body[i - 1];
  s.body[0] = head;
  if (eat && s.len < MAX_LEN) s.len++;
}

void stepSolo() {
  p1.dirX = p1.nextX;
  p1.dirY = p1.nextY;
  bool wallDead;
  Point head = advance(p1, wallDead);
  if (wallDead || hits(p1, head, 1)) {
    gameOver();
    return;
  }
  bool eat = (head.x == food.x && head.y == food.y);
  grow(p1, head, eat);
  if (eat) placeFood();
  if (p1.len >= MAX_LEN) gameOver();
}

void stepMulti() {
  p1.dirX = p1.nextX;
  p1.dirY = p1.nextY;
  p2.dirX = p2.nextX;
  p2.dirY = p2.nextY;

  bool wd1, wd2;
  Point h1 = advance(p1, wd1);
  Point h2 = advance(p2, wd2);

  bool dead1 = wd1 || hits(p1, h1, 1) || hits(p2, h1, 0);
  bool dead2 = wd2 || hits(p2, h2, 1) || hits(p1, h2, 0);
  if (h1.x == h2.x && h1.y == h2.y) {
    dead1 = true;
    dead2 = true;
  }

  if (dead1 || dead2) {
    if (dead1 && dead2) winScreen("Egalite");
    else if (dead1) winScreen("P2 gagne");
    else winScreen("P1 gagne");
    return;
  }

  bool eat1 = (h1.x == food.x && h1.y == food.y);
  bool eat2 = (h2.x == food.x && h2.y == food.y);
  grow(p1, h1, eat1);
  grow(p2, h2, eat2);
  if (eat1 || eat2) placeFood();
}

void startGame() {
  inMenu = false;
  resetGame();
  draw();
}

void applyKey(char c) {
  if (c == '3') {
    gameOver();
    return;
  }
  if (inMenu) {
    if (menuStage == 0) {
      if (c == '1') { selMulti = false; menuStage = 1; showMenu(); }
      else if (c == '2') { selMulti = true; menuStage = 1; showMenu(); }
    } else {
      if (c == '1') { walls = false; twoPlayers = selMulti; startGame(); }
      else if (c == '2') { walls = true; twoPlayers = selMulti; startGame(); }
    }
    return;
  }
  if (c == 'z' && p1.dirY != 1) { p1.nextX = 0; p1.nextY = -1; }
  else if (c == 's' && p1.dirY != -1) { p1.nextX = 0; p1.nextY = 1; }
  else if (c == 'q' && p1.dirX != 1) { p1.nextX = -1; p1.nextY = 0; }
  else if (c == 'd' && p1.dirX != -1) { p1.nextX = 1; p1.nextY = 0; }
  else if (twoPlayers && c == 'i' && p2.dirY != 1) { p2.nextX = 0; p2.nextY = -1; }
  else if (twoPlayers && c == 'k' && p2.dirY != -1) { p2.nextX = 0; p2.nextY = 1; }
  else if (twoPlayers && c == 'j' && p2.dirX != 1) { p2.nextX = -1; p2.nextY = 0; }
  else if (twoPlayers && c == 'l' && p2.dirX != -1) { p2.nextX = 1; p2.nextY = 0; }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, true, false)) {
    for (;;) ;
  }
  randomSeed(esp_random());
  inMenu = true;
  menuStage = 0;
  showMenu();
  lastStep = millis();
}

void loop() {
  while (Serial.available()) applyKey((char)Serial.read());
  if (inMenu) return;

  bool ready = twoPlayers || p1.nextX != 0 || p1.nextY != 0;
  if (ready && millis() - lastStep >= STEP_MS) {
    lastStep = millis();
    if (twoPlayers) stepMulti();
    else stepSolo();
    draw();
  }
}
