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

/* Defines constants for mapping touches to the pixel dimensions,
   these numbers were found by playing around with the x, y values
    before mapping them                                         */
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
#define GAME_WIDTH 5 * SPACE_BETWEEN + 4 * TILE_SIZE
#define BUTTON_COL_WIDTH 100 

#define GAME_TIME 15

// Definces constants used for random board generation
#define NUM_SHUFFLES 30
#define NUM_DICE_SIDES 6

/* 'DICE' is a variable that holds the contents of all 16 boggle dice used in the original game,
   'DICE' contents won't change but the contents will be swapped so it can't be declared const */
String DICE[16] = { "AEANEG", "AHSPCO", "ASPFFK", "OBJOAB", "IOTMUC", "RYVDEL",
                    "LREIXD", "EIUNES", "WNGEEH", "LNHNRZ", "TSTIYD", "OWTOAT",
                    "ERTTYL", "TOESSI", "TERWHV", "NUIHMQ" };

/* Declares a variable that holds the 16 randomly chosen charcters in a game of boggle,
   'boardLetters' can't be declared const as it will change from game to game,
   'boardLetters' is global because it is used very frequently and contents are raraely changed */
String boardLetters;

// Keeps Track of Points and time for the game
uint16_t points = 0;
uint32_t time = GAME_TIME; 

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

//********************Functions used in multiple areas********************//
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

void DrawButton(uint16_t x , uint16_t y, String text) {
    tft.setTextColor(ILI9341_BLACK);
    tft.fillRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_WHITE);
    tft.setCursor(x + ((BUTTON_WIDTH - text.length() * (CHAR_WIDTH + SPACE_BETWEEN_CHARS)) / 2), 
                  y + ((BUTTON_HEIGHT - (CHAR_HEIGHT - SPACE_BETWEEN_CHARS)) / 2));
    tft.print(text);
}

uint8_t TilePress() {
    // Get the touch coordinates
    TSPoint touch = processTouch();
    int8_t tilePressed;

    // If touch wasnt registered then set tile pressed to -1
    if (touch.x == 0 && touch.y == 0)
        tilePressed = -1;
    // Otherwise, Compute the tile that was pressed
    else if (touch.x < 5 * SPACE_BETWEEN + 4 * TILE_SIZE && touch.y < 5 * SPACE_BETWEEN + 4 * TILE_SIZE) {
        uint8_t x = constrain((touch.x - SPACE_BETWEEN) / (TILE_SIZE + SPACE_BETWEEN), 0, 3); // X coordinate
        uint8_t y = constrain((touch.y - SPACE_BETWEEN) / (TILE_SIZE + SPACE_BETWEEN), 0, 3); // Y coordinate
        tilePressed = (x + 4 * y); // Sum X and Y
    }

    return tilePressed;
} 

//********************END OF FUNCTION BLOCK*******************************//




//********************Functions that will be used at the end of the game********************//



//**********************************END OF FUNCTION BLOCK**************************************//





//********************Functions that will be used during gameplay********************//
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

void CheckTime(uint32_t start) {
    if (time - (millis() / 1000 - start) < time) {
        if (time > 10) {
            tft.setCursor(GAME_WIDTH + (BUTTON_COL_WIDTH - 22) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
        }
        else {
            tft.fillRect(GAME_WIDTH + (BUTTON_COL_WIDTH - 22) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20, 
                         24, 16, ILI9341_BLACK);
            tft.setCursor(GAME_WIDTH + (BUTTON_COL_WIDTH - 12) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
        }
        time = constrain(time - (millis() / 1000 - start), 0, GAME_TIME);
        tft.print(time);
    }
}

void InGameSetup() {
    tft.fillRect(4 * TILE_SIZE + 5 * SPACE_BETWEEN, SPACE_BETWEEN, BUTTON_WIDTH,
         DISPLAY_HEIGHT - 2 * SPACE_BETWEEN, ILI9341_BLACK);

    DrawButton(GAME_WIDTH, SPACE_BETWEEN, "SOLVE");
    DrawButton(GAME_WIDTH, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "ERASE");
    DrawButton(GAME_WIDTH, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "ENTER");

    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    
    tft.setCursor(GAME_WIDTH + (BUTTON_COL_WIDTH - 45) / 2, TILE_SIZE + 2 * SPACE_BETWEEN);
    tft.print("TIME");

    tft.setCursor(GAME_WIDTH + (BUTTON_COL_WIDTH - 22) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
    tft.print(time);

    tft.setCursor(GAME_WIDTH + (BUTTON_COL_WIDTH - 70) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN);
    tft.print("POINTS");

    tft.setCursor(GAME_WIDTH + (BUTTON_COL_WIDTH - 10) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN + 20);
    tft.print(points);

    // Generates a random board NUM_SHUFFLES times, this is what creates the "shuffle" animation
    for (int i = 0; i < NUM_SHUFFLES; i++) {
        GenerateLetters();
        boardLetters = "";
    }
}

void InGame() {
    // Draw the in game UI
    InGameSetup();

    while (time > 0) {
        uint32_t startTime = millis() / 1000;
        int8_t tilePressed = TilePress();
        Serial.println(tilePressed);
        CheckTime(startTime);
    }
    Serial.println("END");
    //EndGame();
}

//**********************************************END OF FUNCTION BLOCK*************************************//






//********************Functions that will be used for setup********************//
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

void PreGame() {
    drawStart();
    while (true) {
        // Gets touch screen values
        TSPoint touch = processTouch();
        if (touch.x > 5 * SPACE_BETWEEN + 4 * TILE_SIZE)
            InGame();

    }
}

//********************************END OF FUNCTION BLOCK*********************************//






//********************Functions that will be used in the 'solver' mode********************//



//**********************************END OF FUNCTION BLOCK*********************************//






int main() {

    setup();
    PreGame();
    Serial.end();
    return 0;
}
