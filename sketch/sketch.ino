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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 100000, 100000);

struct Point { int8_t x; int8_t y; };

Point snake[MAX_LEN];
int snakeLen;
Point food;

void draw() {
  display.clearDisplay();
  for (int x = 0; x < SCREEN_WIDTH; x += CELL)
    for (int y = 0; y < SCREEN_HEIGHT; y += CELL)
      display.drawPixel(x, y, SSD1306_WHITE);
  display.drawRect(food.x * CELL, food.y * CELL, CELL, CELL, SSD1306_WHITE);
  for (int i = 0; i < snakeLen; i++)
    display.fillRect(snake[i].x * CELL, snake[i].y * CELL, CELL, CELL, SSD1306_WHITE);
  display.display();
}

void setup() {
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, true, false)) {
    for (;;) ;
  }

  snakeLen = 3;
  snake[0] = { GRID_W / 2, GRID_H / 2 };
  snake[1] = { GRID_W / 2 - 1, GRID_H / 2 };
  snake[2] = { GRID_W / 2 - 2, GRID_H / 2 };

  food = { GRID_W / 2 + 5, GRID_H / 2 };

  draw();
}

void loop() {
}
