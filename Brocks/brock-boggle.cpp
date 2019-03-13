// All nessecary files to include
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TouchScreen.h>

// Define pins for the display
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6
#define RAND_PIN A7

// Define thresholds to determine if there was a touch
#define MIN_PRESSURE 10
#define MAX_PRESSURE 1000
#define TOUCH_X_MIN 160
#define TOUCH_X_MAX 880
#define TOUCH_Y_MIN 100
#define TOUCH_Y_MAX 920

// Define touch screen pins, obtained from the documentaion
#define YP A2  // Must be an analog pin, use "An" notation!
#define XM A3  // Must be an analog pin, use "An" notation!
#define YM  5  // Can be a digital pin
#define XP  4  // Can be a digital pin

// Define the display dimensions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// Defines constants for the tiles (For Formatting the UI)
#define NUM_TILES 16
#define TILE_SIZE 40
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 100
#define SPACE_BETWEEN 10

// Defines constants for the text
#define TEXT_SIZE 3
#define CHAR_HEIGHT TEXT_SIZE * 8
#define CHAR_WIDTH TEXT_SIZE * 5
#define SPACE_BETWEEN_CHARS 2

// Definces constants used for random board generation
#define NUM_SHUFFLES 30
#define NUM_DICE_SIDES 6
String DICE[16] = { "AEANEG", "AHSPCO", "ASPFFK", "OBJOAB", "IOTMUC", "RYVDEL",
                    "LREIXD", "EIUNES", "WNGEEH", "LNHNRZ", "TSTIYD", "OWTOAT",
                    "ERTTYL", "TOESSI", "TERWHV", "NUIHMQ" };
String boardLetters;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/* Multimeter reading says there are 300 ohms of resistance across the plate,
   so initialize with this to get more accurate readings */
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void setup() {
    init();

    // Initialize Serial Monitor
    Serial.begin(9600);

    // Initializes tft
    tft.begin();

    // Sets rotation, text size, text wrap, and cursor position
    tft.setRotation(1);
    tft.setTextWrap(false);
    tft.setTextSize(TEXT_SIZE);

    // Draws a black background
    tft.fillScreen(ILI9341_BLACK);
}

void drawStart() {
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            tft.fillRect((col + 1) * SPACE_BETWEEN + col * TILE_SIZE, (row + 1) * SPACE_BETWEEN + row * TILE_SIZE,
             TILE_SIZE, TILE_SIZE, ILI9341_WHITE);
        }
    }

    tft.fillRect(4 * TILE_SIZE + 5 * SPACE_BETWEEN, SPACE_BETWEEN, BUTTON_WIDTH,
         DISPLAY_HEIGHT - 2 * SPACE_BETWEEN, ILI9341_WHITE);

    tft.setCursor(4 * TILE_SIZE + 5 * SPACE_BETWEEN + 25, DISPLAY_HEIGHT / 2 - CHAR_HEIGHT);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    tft.print("NEW");
    tft.setCursor(4 * TILE_SIZE + 5 * SPACE_BETWEEN + 15, DISPLAY_HEIGHT / 2 + CHAR_HEIGHT / 2);
    tft.print("GAME");
}

TSPoint processTouch() {
    TSPoint touch = ts.getPoint();

     // Checks to see if screen was touched or not
    if (touch.z < MIN_PRESSURE || touch.z > MAX_PRESSURE) {
        touch.x = 0;
        touch.y = 0;
        return touch;
    }

    // Stores the x,y coordinates of touch and maps their value to the display
    int touchX = map(touch.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, DISPLAY_WIDTH);
    int touchY = DISPLAY_HEIGHT - map(touch.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, DISPLAY_HEIGHT);

    touch.x = touchX;
    touch.y = touchY;

    return touch;
}

void GenerateLetters() {
    int dieChosen;
    int letterChosen;

    for (int i = 0; i < NUM_TILES; i++) {
        /* Selects a random dice from the remaining set, selects a random letter from that dice,
           appends it to the string                                                           */
        dieChosen = analogRead(RAND_PIN) % (NUM_TILES - i);
        letterChosen = analogRead(RAND_PIN) % NUM_DICE_SIDES;
        boardLetters += DICE[dieChosen][letterChosen];

        // Draws the letter into its respective spot on the board
        int col = i % 4; // Determines column on board
        int row = i / 4; // Determines row on board
        tft.drawChar((SPACE_BETWEEN * (col + 1)) + (TILE_SIZE * col) + SPACE_BETWEEN + 2, 
                     (SPACE_BETWEEN * (row + 1)) + (TILE_SIZE * row) + SPACE_BETWEEN,
                      boardLetters[i], ILI9341_BLACK, ILI9341_WHITE, TEXT_SIZE);

        // Swaps chosen dice to the back so it can't be chosen again
        String temp = DICE[dieChosen];
        DICE[dieChosen] = DICE[NUM_TILES - (i + 1)];
        DICE[NUM_TILES - (i + 1)] = temp;
    } 
}


void drawButton(int x , int y, String text) {
    tft.setTextColor(ILI9341_BLACK);
    tft.fillRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_WHITE);
    tft.setCursor(x + ((BUTTON_WIDTH - text.length() * (CHAR_WIDTH + SPACE_BETWEEN_CHARS)) / 2), 
                  y + ((BUTTON_HEIGHT - (CHAR_HEIGHT - SPACE_BETWEEN_CHARS)) / 2));
    tft.print(text);
}

void inGameSetup() {
    tft.fillRect(4 * TILE_SIZE + 5 * SPACE_BETWEEN, SPACE_BETWEEN, BUTTON_WIDTH,
         DISPLAY_HEIGHT - 2 * SPACE_BETWEEN, ILI9341_BLACK);

    drawButton(5 * SPACE_BETWEEN + 4 * TILE_SIZE, SPACE_BETWEEN, "SOLVE");
    drawButton(5 * SPACE_BETWEEN + 4 * TILE_SIZE, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "ERASE");
    drawButton(5 * SPACE_BETWEEN + 4 * TILE_SIZE, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "ENTER");

    // Generates a random board NUM_SHUFFLES times
    for (int i = 0; i < NUM_SHUFFLES; i++){
        GenerateLetters();
        boardLetters = "";
    }
}
 

void inGame() {
    inGameSetup();
    //drawButton()
}

void preGame() {
    drawStart();
    while (true) {
        // Gets touch screen values
        TSPoint touch = processTouch();
        Serial.print(touch.x);
        Serial.print(", ");
        Serial.println(touch.y);
        delay(500);
        if (touch.x > 5 * SPACE_BETWEEN + 4 * TILE_SIZE)
            inGame();

    }
}

int main() {

    setup();
    preGame();
    Serial.end();
    return 0;
}
