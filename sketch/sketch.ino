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

Point snake[MAX_LEN];
int snakeLen;
Point food;
int8_t dirX;
int8_t dirY;
int8_t nextX;
int8_t nextY;
bool walls;
bool inMenu;
unsigned long lastStep;

void placeFood() {
  bool onSnake;
  do {
    food.x = random(GRID_W);
    food.y = random(GRID_H);
    onSnake = false;
    for (int i = 0; i < snakeLen; i++)
      if (snake[i].x == food.x && snake[i].y == food.y) { onSnake = true; break; }
  } while (onSnake);
}

void resetGame() {
  snakeLen = 3;
  snake[0] = { GRID_W / 2, GRID_H / 2 };
  snake[1] = { GRID_W / 2 - 1, GRID_H / 2 };
  snake[2] = { GRID_W / 2 - 2, GRID_H / 2 };
  dirX = 1;
  dirY = 0;
  nextX = 0;
  nextY = 0;
  placeFood();
}

void drawMenu() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(22, 6);
  display.print("SNAKE");
  display.setTextSize(1);
  display.setCursor(12, 36);
  display.print("1: Traverser");
  display.setCursor(12, 50);
  display.print("2: Murs");
  display.display();
}

void gameOver() {
  inMenu = true;
  drawMenu();
}

void draw() {
  display.clearDisplay();
  if (walls)
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.drawRect(food.x * CELL, food.y * CELL, CELL, CELL, SSD1306_WHITE);
  for (int i = 0; i < snakeLen; i++)
    display.fillRect(snake[i].x * CELL, snake[i].y * CELL, CELL, CELL, SSD1306_WHITE);
  display.display();
}

void step() {
  dirX = nextX;
  dirY = nextY;

  int nx = snake[0].x + dirX;
  int ny = snake[0].y + dirY;
  if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) {
    if (walls) {
      gameOver();
      return;
    }
    if (nx < 0) nx = GRID_W - 1;
    else if (nx >= GRID_W) nx = 0;
    if (ny < 0) ny = GRID_H - 1;
    else if (ny >= GRID_H) ny = 0;
  }
  Point next = { (int8_t)nx, (int8_t)ny };

  for (int i = 0; i < snakeLen - 1; i++)
    if (snake[i].x == next.x && snake[i].y == next.y) {
      gameOver();
      return;
    }

  bool eat = (next.x == food.x && next.y == food.y);

  for (int i = snakeLen; i > 0; i--) snake[i] = snake[i - 1];
  snake[0] = next;
  if (eat) {
    if (snakeLen < MAX_LEN) snakeLen++;
    placeFood();
  }

  if (snakeLen >= MAX_LEN) gameOver();
}

void applyKey(char c) {
  if (inMenu) {
    if (c == '1') { walls = false; inMenu = false; resetGame(); draw(); }
    else if (c == '2') { walls = true; inMenu = false; resetGame(); draw(); }
    return;
  }
  if (c == 'z' && dirY != 1) { nextX = 0; nextY = -1; }
  else if (c == 's' && dirY != -1) { nextX = 0; nextY = 1; }
  else if (c == 'q' && dirX != 1) { nextX = -1; nextY = 0; }
  else if (c == 'd' && dirX != -1) { nextX = 1; nextY = 0; }
  else if (c == '1' || c == '2') gameOver();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, true, false)) {
    for (;;) ;
  }
  randomSeed(esp_random());
  inMenu = true;
  drawMenu();
  lastStep = millis();
}

void loop() {
  while (Serial.available()) applyKey((char)Serial.read());

  if (!inMenu && (nextX != 0 || nextY != 0) && millis() - lastStep >= STEP_MS) {
    lastStep = millis();
    step();
    draw();
  }
}
