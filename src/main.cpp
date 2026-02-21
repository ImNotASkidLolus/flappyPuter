#include <Arduino.h>
#include <M5Cardputer.h>
#include <list>

#define m M5Cardputer

struct Rectangle {
    float x;
    float y;
    float width;
    float height;
};

struct Vector2 {
    float x;
    float y;
};

class Obstacle{
    public:
        Rectangle rect;
        Obstacle(Rectangle rect) {
            this->rect = rect;
        }
};

//stolen from raylib.h library for cpp
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)
{
    bool collision = false;

    if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
        (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y)) collision = true;

    return collision;
}

std::list<Obstacle> pipeUp;
std::list<Obstacle> pipeDown;

Rectangle position;
Vector2 velocity = {0, 0};
const float gravity = 2.0;

int jumpTimer = 1;
bool jump = false;
bool gameOver = false;
bool start = false;
bool passed = false;
int score = 0;
float beforeTime = 0;
int gap = 50;

void setup() {
    m.begin();
    m.Display.fillScreen(TFT_BLACK);
    int w = m.Lcd.width();
    int h = m.Lcd.height();

    const float randomNum = random(h / 5, h - gap);
    pipeUp.emplace_back(Obstacle({(float)w, 0, 40, randomNum}));
    pipeDown.emplace_back(Obstacle({(float)w, (float)h - randomNum, 40, h - randomNum}));

    position = {(float)m.Lcd.width() / 2, (float)m.Lcd.height() / 2, 30, 30};


    m.Display.setTextColor(TFT_WHITE);
    m.Display.setBrightness(70);
    m.Display.setCursor(w/ 2 - 40, h / 2);
    m.Display.print("ENTER -> START");
    m.Display.setCursor(0, 0);
}

void loop() {
    float currentTime = millis();
    float deltaTime = (currentTime - beforeTime) / 1000;
    beforeTime = currentTime;
    M5Canvas c(&m.Lcd);
    m.update();


    int w = m.Lcd.width();
    int h = m.Lcd.height();
    c.createSprite(w, h);

    if (gameOver) {
        score = 0;
        m.Display.setTextColor(TFT_BLACK);
        m.Display.setTextSize(3);
        m.Display.setCursor(w /4 - 10, h/ 2);
        m.Display.print("GAME OVER");
        if (m.Keyboard.isChange()) {
            if (m.Keyboard.isKeyPressed(KEY_ENTER)) {
                position.y = h / 2;
                velocity.y = 0;
                pipeUp.clear();
                pipeDown.clear();
                float randomHeight = random(h / 5, h - gap - 10);
                pipeUp.emplace_back(Obstacle({(float)w, 0, 40, randomHeight}));
                pipeDown.emplace_back(Obstacle({(float)w, randomHeight + gap, 40, randomHeight + gap}));
                gameOver = false;
                passed = false;
            }
        }
    }else {
        if (!start) {
            c.fillSprite(TFT_BLACK);
            m.Display.setTextColor(TFT_WHITE);
            m.Display.setCursor(w / 2 - 40, h / 2);
            m.Display.print("ENTER -> START");

            if (m.Keyboard.isChange()) {
                if (m.Keyboard.isKeyPressed(KEY_ENTER)) {
                    start = true;
                    passed = false;
                }
            }
        }else {
            c.fillSprite(TFT_DARKCYAN);

            for (auto& pipes :pipeUp) {
                c.fillRect(pipes.rect.x, pipes.rect.y, pipes.rect.width, pipes.rect.height, TFT_GREEN);
            }
            for (auto& pipes :pipeDown) {
                c.fillRect(pipes.rect.x, pipes.rect.y, pipes.rect.width, pipes.rect.height, TFT_GREEN);
            }
            c.setTextSize(3);
            c.setTextColor(TFT_BLACK);
            c.print(score);
            c.fillRect(position.x, position.y, position.width, position.height, TFT_RED);
            c.pushSprite(0, 0);

            if (pipeDown.back().rect.x <= w - 100) {
                float randomHeight = random(h / 5, h - gap - 10);
                pipeDown.emplace_back(Obstacle({(float)w, randomHeight + gap, 40, randomHeight + gap}));
                pipeUp.emplace_back(Obstacle({(float)w, 0, 40, randomHeight}));
                passed = false;
            }
            if (pipeDown.front().rect.x <= -pipeDown.front().rect.width) {
                pipeDown.pop_front();
                pipeUp.pop_front();
            }


            if (jump) {
                velocity.y = -45;
                jump = false;
            }

            velocity.y += gravity;
            position.y += velocity.y * deltaTime;
            for (auto& pipes : pipeUp) {
                pipes.rect.x -= 30 * deltaTime ;
                if (pipes.rect.x + position.width < position.x && passed == false) {
                    score++;
                    passed = true;
                }
            }
            for (auto& pipes : pipeDown) {
                pipes.rect.x -= 30 * deltaTime;
            }

            if (position.y >= h) {
                position.y = h;
                velocity.y = 0;
                gameOver = true;
            }
            if (position.y <= 0) {
                position.y = 0;
                gameOver = true;
            }
            if (m.Keyboard.isChange()) {
                if (m.Keyboard.isKeyPressed(KEY_ENTER)) {
                    jump = true;
                }
            }
            for (auto& pipes : pipeDown) {
                if (pipes.rect.x - position.width <= position.x ) {
                    if (CheckCollisionRecs(pipes.rect, position)) {
                        gameOver = true;
                    }
                }
            }
            for (auto& pipes : pipeUp) {
                if (pipes.rect.x - position.width <= position.x ) {
                    if (CheckCollisionRecs(pipes.rect, position)) {
                        gameOver = true;
                    }
                }
            }
        }
    }
}