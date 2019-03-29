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
#define SOUND_PIN 3

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
#define SPACE_BETWEEN_CHARS TEXT_SIZE * 1


#define GAME_SIZE 5 * SPACE_BETWEEN + 4 * TILE_SIZE
#define BUTTON_COL_WIDTH 100 

#define GAME_TIME 60

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

bool pressPrev = false;
bool pressCurr = false;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/* Multimeter reading says there are 300 ohms of resistance across the plate,
   so initialize with this to get more accurate readings */
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void InGame(bool newLetters);

void setup() {
    init();

    // Initialize Serial Monitor
    Serial.begin(9600);

    Serial.flush();

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

        pressPrev = false;
        pressCurr = false;
        return touch;
    }

    // Stores the x,y coordinates of touch and maps their value to the display
    int touchX = map(touch.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, DISPLAY_WIDTH);
    int touchY = DISPLAY_HEIGHT - map(touch.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, DISPLAY_HEIGHT);

    touch.x = touchX;
    touch.y = touchY;

    pressCurr = true;

    return touch;
}

void DrawButton(uint16_t x , uint16_t y, String text) {
    // Draws a button starting at x, y coordinate. Width is 100, Height is 30
    tft.setTextColor(ILI9341_BLACK);
    tft.fillRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_WHITE);
    tft.setCursor(x + ((BUTTON_WIDTH - text.length() * (CHAR_WIDTH + SPACE_BETWEEN_CHARS)) / 2), 
                  y + ((BUTTON_HEIGHT - (CHAR_HEIGHT - SPACE_BETWEEN_CHARS)) / 2));
    tft.print(text);
}

void DrawTile(char letter, uint8_t index, uint16_t bgColor) {
    // Draws a tile with a specific letter in it and specifies color
    uint16_t x = SPACE_BETWEEN * ((index % 4) + 1) + TILE_SIZE * (index % 4);
    uint16_t y = SPACE_BETWEEN * ((index / 4) + 1) + TILE_SIZE * (index / 4);

    tft.fillRect(x, y, TILE_SIZE, TILE_SIZE, bgColor);
    tft.drawChar(x + SPACE_BETWEEN + 2, y + SPACE_BETWEEN, letter, ILI9341_BLACK, bgColor, TEXT_SIZE); 
}

void DrawWord(String word) {
    // Fill in the rectangle
    tft.fillRect(0, GAME_SIZE, GAME_SIZE, DISPLAY_HEIGHT - GAME_SIZE, ILI9341_BLACK);
    
    // Reset the cursor
    tft.setCursor(SPACE_BETWEEN, GAME_SIZE);

    // Write the word
    tft.print(word);

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
}

void ConnectTiles(uint8_t prev, uint8_t curr, uint8_t wLength, uint16_t color) {
    // 4 different types of connections (Horizontal, Vertical, Diag Up Right, Diag Down Right)
    uint8_t minX = min(curr % 4, prev % 4);
    uint8_t minY = min(curr / 4, prev / 4);

    uint8_t maxX = max(curr % 4, prev % 4);
    uint8_t maxY = max(curr / 4, prev / 4);

    // The Vertical Case
    if (curr % 4 == prev % 4)
        tft.drawFastVLine((minX + 1) * SPACE_BETWEEN + (minX + 0.5)* TILE_SIZE, 
                         (minY + 1) * SPACE_BETWEEN + (minY + 1) * TILE_SIZE, SPACE_BETWEEN, color);
    // The Horizontal Case
    else if (curr / 4 == prev / 4) 
        tft.drawFastHLine((minX+ 1) * SPACE_BETWEEN + (minX + 1) * TILE_SIZE,
                          (minY + 1) * SPACE_BETWEEN + (minY + 0.5) * TILE_SIZE, SPACE_BETWEEN, color);
    // The Down Right / Up left Case
    else if ((curr % 4 < prev % 4 && curr / 4 < prev / 4) || (prev % 4 < curr % 4 && prev / 4 < curr / 4))
        tft.drawLine((minX + 1) * SPACE_BETWEEN + (minX + 1) * TILE_SIZE, (minY + 1) * SPACE_BETWEEN + (minY + 1) * TILE_SIZE,
                    (maxX + 1) * SPACE_BETWEEN + maxX * TILE_SIZE, (maxY + 1) * SPACE_BETWEEN + maxY * TILE_SIZE, color);
    // The Down Left / Up Right Case
    else 
        tft.drawLine((minX + 1) * SPACE_BETWEEN + (minX + 1) * TILE_SIZE, (maxY + 1) * SPACE_BETWEEN + maxY * TILE_SIZE,
                    (maxX + 1) * SPACE_BETWEEN + maxX * TILE_SIZE, (minY + 1) * SPACE_BETWEEN + (minY + 1) * TILE_SIZE, color);
}

void BoxStartTile(uint8_t tile) {
    for (int i = 0; i < 5; i++) {
        tft.drawRect(((tile % 4) + 1) * SPACE_BETWEEN + (tile % 4) * TILE_SIZE + i,
                    ((tile / 4) + 1) * SPACE_BETWEEN + (tile / 4) * TILE_SIZE + i,
                    TILE_SIZE - 2*i, TILE_SIZE - 2*i, ILI9341_RED);
    }
}

void ClearBoard() {
    tft.fillRect(0,0, GAME_SIZE, DISPLAY_HEIGHT, ILI9341_BLACK);
    
    for (int i = 0; i < NUM_TILES; i++)
        DrawTile(boardLetters[i], i, ILI9341_WHITE);
}

//********************END OF FUNCTION BLOCK*******************************//



//********************Functions that will be used in the 'solver' mode********************//
String ReadToSpace() {
    /*
    DESCRIPTION
    Reads all letters and stores up to a string

    PARAMETERS
    Void

    RETURNS
    line (String): A string storing all characters up to a space or \n
    */

    // Declares a letter and a line
    char letter;
    String line = "";

    // If top bit on serial monitor is acknowledgement, keep looking
    while (Serial.peek() == '*' || Serial.available() == 0) {}

    while (true) {
        // Read in a character
        letter = Serial.read();

        // If newline is reached then return
        if (letter == '\n') {
            Serial.println('*'); // * means acknowledgement
            Serial.flush();
            delay(10);
            return line; // Return the line
        }

    // Add letter to line
    line += letter;
    }
}

void GetAndDrawWord() {
    uint8_t length = ReadToSpace().toInt();
    
    String word = "";
    // Declares variables to hold current and previous space of tiles
    uint8_t prevTile;
    uint8_t tile;

    // Get as many tiles as there are letters in the word
    for (int i = 0; i < length; i++) {
        tile = ReadToSpace().toInt();
        word += boardLetters[tile];

        DrawTile(boardLetters[tile], tile, ILI9341_GREEN);

        if (i == 0)
            BoxStartTile(tile);
        else
            ConnectTiles(prevTile, tile, word.length(), ILI9341_GREEN);

        prevTile = tile;
    }



    // Reset the word box
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    DrawWord(word);
}

void RedrawIndex(uint16_t i) {
    tft.fillRect(GAME_SIZE + 60, 2 * BUTTON_HEIGHT + 3 * SPACE_BETWEEN, BUTTON_WIDTH - 60, 16, ILI9341_BLACK);
    tft.setCursor(GAME_SIZE + 60, 2 * BUTTON_HEIGHT + 3 * SPACE_BETWEEN);
    tft.print(i);
}

uint16_t SolveBoardSetup() {
    // Clears the Screen
    tft.fillRect(GAME_SIZE, 0, BUTTON_WIDTH, DISPLAY_HEIGHT, ILI9341_BLACK);
    ClearBoard();

    // Draws the 4 solving mode buttons
    tft.setTextSize(TEXT_SIZE);
    DrawButton(GAME_SIZE, SPACE_BETWEEN, "PREV");
    DrawButton(GAME_SIZE, 2 * SPACE_BETWEEN + BUTTON_HEIGHT, "NEXT");
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "NEW");
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "SAME");

    tft.setCursor(GAME_SIZE, 2 * BUTTON_HEIGHT + 3 * SPACE_BETWEEN);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print("WORD:"); 
    RedrawIndex(1);

    tft.setCursor(GAME_SIZE, 2 * BUTTON_HEIGHT + 5 * SPACE_BETWEEN);
    tft.print("OF:");

    uint16_t numWords = ReadToSpace().toInt();
    tft.setCursor(GAME_SIZE + 38, 2 * BUTTON_HEIGHT + 5 * SPACE_BETWEEN);
    tft.print(numWords);

    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(GAME_SIZE, 2 * BUTTON_HEIGHT + 8 * SPACE_BETWEEN);
    tft.print("PP:");

    uint16_t possiblePoints = ReadToSpace().toInt();
    tft.setCursor(GAME_SIZE + 38, 2 * BUTTON_HEIGHT + 8 * SPACE_BETWEEN);
    tft.print(possiblePoints);

    return numWords;
}

uint16_t SolvingButtons(TSPoint touch, uint16_t index, uint16_t numWords) {
    if (touch.x < GAME_SIZE)
        return index;

    // Check if same was pressed
    if (touch.y > GAME_SIZE - SPACE_BETWEEN) {
        Serial.println("0"); // 0 indicates new game, same board
        ClearBoard();

        // Go back into the game, without generating new letters
        InGame(false);
    }
    else if (touch.y > GAME_SIZE - TILE_SIZE - SPACE_BETWEEN) {
        Serial.println("1"); // 0 indicates new game, same board
        ClearBoard();
        // Go back into the game, without generating new letters
        InGame(true);
    }
    else if (touch.y < 1 * BUTTON_HEIGHT + 1 * SPACE_BETWEEN && index > 0) {
        index--;
        RedrawIndex(index + 1);
        ClearBoard();
        Serial.println("2");
        GetAndDrawWord();
    }
    else if (touch.y < 2 * BUTTON_HEIGHT + 2 * SPACE_BETWEEN && touch.y > SPACE_BETWEEN + BUTTON_HEIGHT
            && index < numWords - 1) {
        index++;
    RedrawIndex(index + 1);
        ClearBoard();
        Serial.println("3");
        GetAndDrawWord();
    }

    return index;
}

void SolveBoard() {
    uint16_t numWords = SolveBoardSetup();

    GetAndDrawWord();

    uint16_t index = 0;
    while (true) {
        TSPoint touch = processTouch();

        if (!pressCurr || pressPrev)
            continue;

        index = SolvingButtons(touch, index, numWords);
    }
}


//**********************************END OF FUNCTION BLOCK*********************************//



//********************Functions that will be used at the end of the game********************//

void PostGameSetup() {
    // Draws the 3 post game buttons
    tft.setTextSize(TEXT_SIZE);
    DrawButton(GAME_SIZE, SPACE_BETWEEN, "SOLVE");
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "NEW");
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "SAME");

    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    DrawWord("GAME OVER!");
}

void PostGameButtons(TSPoint touch) {
    if (touch.x < GAME_SIZE)
        return;

    time = GAME_TIME;
    points = 0;

    if (touch.y > GAME_SIZE - SPACE_BETWEEN) {
            Serial.println("0"); // 0 indicates new game, same board

            // Go back into the game, without generating new letters
            InGame(false);
    }
    else if (touch.y > GAME_SIZE - TILE_SIZE - SPACE_BETWEEN) {
        Serial.println("1"); // 1 indicates new game, new board

        // Go back into the game, without generating new letters
        InGame(true);
    }
    else if (touch.y < SPACE_BETWEEN + BUTTON_HEIGHT) {
        Serial.println("2"); // 2 indicates enter solving mode

        // Solve the board
        SolveBoard();
    }
}

void PostGame() {
    PostGameSetup();

    while (true) {
        // Get touch coordinates
        TSPoint touch = processTouch();

        if (!pressCurr || pressPrev)
            continue;

        PostGameButtons(touch);
    }
}

//**********************************END OF FUNCTION BLOCK**************************************//





//********************Functions that will be used during gameplay********************//
void GenerateLetters() {
    // Variables that will hold pseudorandomly drawn letters
    int dieChosen;
    int letterChosen;

    // Randomly seeds the input based on the voltage at analog pin
    randomSeed(analogRead(RAND_PIN));

    for (int i = 0; i < NUM_TILES; i++) {
        /* Selects a random dice from the remaining set, selects a random letter from that dice,
           appends it to the string                                                           */
        dieChosen = random(NUM_TILES - i);
        letterChosen = random(NUM_DICE_SIDES);
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

uint8_t TilePress(TSPoint touch) {
    int8_t tilePressed;

    // Otherwise, Compute the tile that was pressed
    if (touch.x < GAME_SIZE && touch.y < GAME_SIZE) {
        uint8_t x = constrain((touch.x) / (TILE_SIZE + SPACE_BETWEEN), 0, 3); // X coordinate
        uint8_t y = constrain((touch.y) / (TILE_SIZE + SPACE_BETWEEN), 0, 3); // Y coordinate
        tilePressed = (x + 4 * y); // Sum X and Y
    }
    else
        return NUM_TILES;

    return tilePressed;
} 

uint32_t CheckTime(uint32_t start) {
    float currTime = millis() / 1000; 
    if (time - (currTime - start) != time) {
        if (time > 10) {
            tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 22) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
        }
        else {
            tft.fillRect(GAME_SIZE + (BUTTON_COL_WIDTH - 22) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20, 
                         24, 16, ILI9341_BLACK);
            tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 12) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
        }
        time = time - (millis() / 1000 - start);
        tft.print(time);
        
        return currTime;
    }
    return start;
}


uint8_t TileValidity(uint8_t* visited, uint8_t tile ,uint8_t wLength) {
    if (wLength == 0)
        return tile;

    // Check 1, is the tile already in the list of visited tiles
    for (int i = 0; i < wLength; i++) {
        if (tile == visited[i]) {
            return NUM_TILES;
        }
    }

    int prev = visited[wLength - 1];

    if (abs((tile % 4) - (prev % 4)) > 1 || abs((tile / 4) - (prev / 4)) > 1)
        return NUM_TILES;

    return tile;
}

void RedrawPoints() {
    if (points < 10)
        tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 10) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN + 20);
    else if (points < 100)
        tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 22) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN + 20);
    else
        tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 34) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN + 20);
    
    tft.print(points);
}

String Erase(String word, uint8_t* visited) {
    // Initialize a new string that will store all but the last letter of word
    String wordDelete = "";

    // Add all but last letter
    for (int i = 0; i < word.length() - 1; i++) {
        wordDelete += word[i];
    }

    // Write the trimmed word, unhighlight the tile, unconnect the tiles, remove from visited
    DrawWord(wordDelete);
    DrawTile(word[word.length() - 1], visited[word.length() - 1], ILI9341_WHITE);

    // If more than one tile is on the board then erase the connection between the previous tile
    if (word.length() > 1)
        ConnectTiles(visited[word.length() - 2], visited[word.length() - 1], word.length(), ILI9341_BLACK);

    visited[word.length() - 1] = NUM_TILES;

    return wordDelete;
}

String Enter(String word, uint8_t* visited) {
    char isWord;

    // Send the word to the server
    Serial.println(word);

    while (Serial.peek() != '0' && Serial.peek() != '1') {}
    // Wait for either a 0 or 1 to be sent
    isWord = Serial.read();
    if (isWord == '1') {
        points += constrain(word.length() - 2, 0, 6);
        RedrawPoints();
    }

    // Clear the word and reset the visited tiles
    while (word != "") 
        word = Erase(word, &visited[0]);

    if (isWord == '1') {
        tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
        DrawWord("VALID WORD!");
    }
    else {
        tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
        DrawWord("INVALID!");
    }
    Serial.println("*");

    // Return the blank string
    return word;
}

String InGameButtons(TSPoint touch, String word, uint8_t* visited) {
    // Determines if somewhere on the buttons column was touched
    if (touch.x > GAME_SIZE) {
        // Determines if the enter button was pressed
        if (touch.y > GAME_SIZE - SPACE_BETWEEN && word.length() > 0)
            word = Enter(word, &visited[0]); 

        // Determines if the erase button was pressed
        else if (touch.y > GAME_SIZE - TILE_SIZE - SPACE_BETWEEN && word.length() > 0) 
            word = Erase(word, &visited[0]);

        // Determines if the solving button was pressed
        else if (touch.y < BUTTON_HEIGHT + SPACE_BETWEEN) {
            Serial.println("2");
            ClearBoard();
            SolveBoard();
        }

        // Delay by 200 milliseconds so it doesnt erase all at once
        delay(200);
    }

    // Return the altered word
    return word;
}

void InGameSetup(bool newLetters) {
    // Clears the word box in the bottom left
    DrawWord("");

    // Sets the text size to 3
    tft.setTextSize(TEXT_SIZE);

    // Clears the Button Column
    tft.fillRect(GAME_SIZE, 0, BUTTON_WIDTH, DISPLAY_HEIGHT, ILI9341_BLACK);

    // Draws the 3 in game buttons
    DrawButton(GAME_SIZE, SPACE_BETWEEN, "SOLVE");
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "ERASE");
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "ENTER");

    // Alters text size, color for writing points and time
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    // Initializes the start time and starting amount of points
    time = GAME_TIME;
    points = 0;
    
    // Write the time indicator as well as the starting time
    tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 45) / 2, TILE_SIZE + 2 * SPACE_BETWEEN);
    tft.print("TIME");
    tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 22) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
    tft.print(time);

    // Write thr points indicator as well as the starting points (0)
    tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 70) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN);
    tft.print("POINTS");
    tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 10) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN + 20);
    tft.print(points);

    // If we require new letters, then new letters will be generated and sent to server
    if (newLetters) {
        // Generates a random board NUM_SHUFFLES times, this is what creates the "shuffle" animation
        for (int i = 0; i < NUM_SHUFFLES; i++) {
            boardLetters = ""; // All but the last shuffle matter
            GenerateLetters(); // Generates 16 random letters for the board
        }
        // Send the board to the server
        Serial.println(boardLetters);
    }
}

void InGame(bool newLetters) {
    // Draw the in game UI
    InGameSetup(newLetters);

    // Initialize list to 16 invalid tile numbers, string to blank
    uint8_t visited[NUM_TILES] = {NUM_TILES};
    String word = "";

    float startTime = millis() / 1000;

    while (time > 0) {
        // Check if the timer needs to decrement by a second
        startTime = CheckTime(startTime);

        // Get touch coordinates
        TSPoint touch = processTouch();

        if (!pressCurr || pressPrev)
            continue;

        // Find which tile, if any was pressed, then check if valid press
        uint8_t tilePressed = TilePress(touch);
        word = InGameButtons(touch, word, &visited[0]);

        tilePressed = TileValidity(&visited[0], tilePressed, word.length());
        
        // If a tile was pressed, draw it
        if (tilePressed < NUM_TILES) {
            // Append the letter to the word being built
            word += boardLetters[tilePressed];

            // Add the tile index to visited tiles
            visited[word.length() - 1] = tilePressed;
            
            // Write the word and highlight the tile pressed
            DrawWord(word);
            DrawTile(word[word.length() - 1], tilePressed, ILI9341_GREEN);

            if (word.length() > 1)
                // Connect tiles if more than one has been pressed
                ConnectTiles(visited[word.length() - 2], tilePressed, word.length(), ILI9341_GREEN);
            else 
                // Box the tile in red if only that tile has been pressed
                BoxStartTile(tilePressed);
        }

        pressPrev = pressCurr;
    }
    word = Enter(word, &visited[0]);
    tone(SOUND_PIN, 800, 1000);
    // Send NEXT PHASE\n to the server to indicate the end of the game
    Serial.println("NEXT PHASE");
    PostGame();
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
            InGame(true);

    }
}

//********************************END OF FUNCTION BLOCK*********************************//


int main() {

    setup();
    PreGame();
    Serial.end();
    return 0;
}