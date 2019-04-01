/*
Names: Brock Chelle, Benjamin Wagg
IDs: 1533398, 1531566
CCID: bchelle, bwagg
Course: CMPUT 275
Term: Winter 2019
Final Project: Boggle Solver (Part 2)
*/

// All nessecary files to include
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TouchScreen.h>

// Defines pins for the display
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

// Defines pins for random number generation and making noise
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

// Defines touch screen pins, obtained from the documentaion
#define YP A2  // Must be an analog pin, use "An" notation!
#define XM A3  // Must be an analog pin, use "An" notation!
#define YM  5  // Can be a digital pin
#define XP  4  // Can be a digital pin

// Defines the display dimensions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// Defines constants for the tiles (For Formatting the UI)
#define NUM_TILES 16
#define TILE_SIZE 40
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 100
#define SPACE_BETWEEN 10
#define GAME_SIZE 5 * SPACE_BETWEEN + 4 * TILE_SIZE
#define BUTTON_COL_WIDTH 100 

// Defines constants for the text
#define TEXT_SIZE 3
#define CHAR_HEIGHT TEXT_SIZE * 8
#define CHAR_WIDTH TEXT_SIZE * 5
#define SPACE_BETWEEN_CHARS TEXT_SIZE * 1

// Defines the amount of time for each game
#define GAME_TIME 100
#define MIL_PER_SEC 1000

// Defines constants used for random board generation
#define NUM_SHUFFLES 30
#define NUM_DICE_SIDES 6

/* Declares a variable that holds the 16 randomly chosen charcters in a game of boggle,
   'boardLetters' can't be declared const as it will change from game to game,
   'boardLetters' is global because it is used very frequently and contents are raraely changed */
String boardLetters;

// Keeps Track of Points and time for the game
uint16_t points;
uint32_t time;

// Keeps track of the current and previous state of the touch screen
bool pressPrev = false;
bool pressCurr = false;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/* Multimeter reading says there are 300 ohms of resistance across the plate,
   so initialize with this to get more accurate readings */
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Predeclared since it is called from many functions
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
    /*
    PURPOSE
    Gets coordinates for a touch on the screen

    RETURNS
    touch (TSPoint): Variable holding x, y coordinates for a given touch
    */

    // Gets coordinates
    TSPoint touch = ts.getPoint();

     // Checks to see if screen was touched or not
    if (touch.z < MIN_PRESSURE || touch.z > MAX_PRESSURE) {
        // Set x and y coordinates to 0
        touch.x = 0; 
        touch.y = 0;

        // Sets both touch states to false
        pressPrev = false;
        pressCurr = false;

        // Returns the touch data
        return touch;
    }

    // Stores the x,y coordinates of touch and maps their value to the display
    uint16_t touchX = map(touch.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, DISPLAY_WIDTH);
    uint16_t touchY = DISPLAY_HEIGHT - map(touch.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, DISPLAY_HEIGHT);

    // Reassignes the mapped coordinates
    touch.x = touchX;
    touch.y = touchY;

    // Sets the current press state to true
    pressCurr = true;

    // Returns the touch data
    return touch;
}

void DrawButton(uint16_t x , uint16_t y, String text) {
    /*
    PURPOSE
    Draws a button with a specified text in it

    PARAMETERS
    x (uint16_t): The x coordinate of the button
    y (uint16_t): The y coordinate of the button
    text (String): The text to place within the button
    */

    // Draws a button starting at x, y coordinate. Width is 100, Height is 30
    tft.fillRect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_WHITE); // Draws button
        
    // Sets text color, centers the cursor in the button and prints the text
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(x + ((BUTTON_WIDTH - text.length() * (CHAR_WIDTH + SPACE_BETWEEN_CHARS)) / 2), 
                  y + ((BUTTON_HEIGHT - (CHAR_HEIGHT - SPACE_BETWEEN_CHARS)) / 2));
    tft.print(text);
}

void DrawTile(char letter, uint8_t index, uint16_t bgColor) {
    /*
    PURPOSE
    Draws a tile of a certain letter at a certain index of a certain color

    PARAMETERS
    letter (char): The letter to be drawn in the tile
    index (uint8_t): The index of the tile
    bgColor (uint16_t): The background color of the tile
    */

    // Calculates the x and y pixel coordinates based on index
    uint16_t x = SPACE_BETWEEN * ((index % 4) + 1) + TILE_SIZE * (index % 4);
    uint16_t y = SPACE_BETWEEN * ((index / 4) + 1) + TILE_SIZE * (index / 4);

    // Fills the index and draws the character in it
    tft.fillRect(x, y, TILE_SIZE, TILE_SIZE, bgColor);
    tft.drawChar(x + SPACE_BETWEEN + 2, y + SPACE_BETWEEN, letter, ILI9341_BLACK, bgColor, TEXT_SIZE); 
}

void DrawWord(String word) {
    /*
    PURPOSE
    Draws the word in the bottom left corner

    PARAMETERS
    word (String): The word to be drawn
    */

    // Fill in the rectangle
    tft.fillRect(0, GAME_SIZE, GAME_SIZE, DISPLAY_HEIGHT - GAME_SIZE, ILI9341_BLACK);
    
    // Reset the cursor
    tft.setCursor(SPACE_BETWEEN, GAME_SIZE);

    // Write the word
    tft.print(word);

    // Reset the text color
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
}

void ConnectTiles(uint8_t prev, uint8_t curr, uint16_t color) {
    /*
    PURPOSE
    Connect two adjacent tiles on the boards

    PARAMETERS
    prev (uint8_t): The index of the previous tile
    curr (uint8_t): The index of the current tile
    color (uint16_t): The color of the connection line
    */

    // 4 different types of connections (Horizontal, Vertical, Diag Up Right, Diag Down Right)

    // Find the minX and minY
    uint8_t minX = min(curr % 4, prev % 4);
    uint8_t minY = min(curr / 4, prev / 4);

    // Find the maxX and the maxY
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
    /*
    PURPOSE
    Box the starting tile with 5 thin bordered squares

    PARAMETERS
    tile (uint8_t): The index of the tile that needs to be boxed
    */

    // Draws 5 rectangles around the tile for better highlighting
    for (int i = 0; i < 5; i++) {
        tft.drawRect(((tile % 4) + 1) * SPACE_BETWEEN + (tile % 4) * TILE_SIZE + i,
                    ((tile / 4) + 1) * SPACE_BETWEEN + (tile / 4) * TILE_SIZE + i,
                    TILE_SIZE - 2*i, TILE_SIZE - 2*i, ILI9341_RED);
    }
}

void ClearBoard() {
    /*
    PURPOSE
    Clears the board
    */

    // Fills the left side of the screen where the board is
    tft.fillRect(0,0, GAME_SIZE, DISPLAY_HEIGHT, ILI9341_BLACK);
    
    // Redraws all the tiles
    for (int i = 0; i < NUM_TILES; i++)
        DrawTile(boardLetters[i], i, ILI9341_WHITE);
}

//********************END OF FUNCTION BLOCK*******************************//



//********************Functions that will be used in the 'solver' mode********************//

String ReadToNewLine() {
    /*
    PURPOSE
    Reads all letters and stores up to a \n character

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
            Serial.flush(); // Wait for * to be sent
            delay(10); // Small delay for better communication
            return line; // Return the line
        }

    // Add letter to line
    line += letter;
    }
}

void GetAndDrawWord() {
    /*
    PURPOSE
    Gets the length (n) of of a word and then n tiles and draws them on the board 
    */

    // Gets the length of the word received
    uint8_t length = ReadToNewLine().toInt();

    // Declares variables to hold current and previous space of tiles
    uint8_t prevTile;
    uint8_t tile;

    // Sets the word to blank
    String word = "";

    // Get as many tiles as there are letters in the word
    for (int i = 0; i < length; i++) {
        // Gets the index of a tile
        tile = ReadToNewLine().toInt();

        // Adds that tiles letter to the string
        word += boardLetters[tile];

        // Highlights the tile
        DrawTile(boardLetters[tile], tile, ILI9341_GREEN);

        // Boxes the tile if it is the first one
        if (i == 0)
            BoxStartTile(tile);
        // Otherwise, connects the current tile and the previous tile
        else
            ConnectTiles(prevTile, tile, ILI9341_GREEN);

        // Sets the previous tile to the current tile
        prevTile = tile;
    }



    // Reset the word box
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    DrawWord(word);
}

void RedrawIndex(uint16_t i) {
    /*
    PURPOSE
    Redraws the 'word index' indicator

    PARAMETERS
    i (uint16_t): The index to be drawn
    */

    // Clears the previous index, sets the cursor and prints the new index
    tft.fillRect(GAME_SIZE + 60, 2 * BUTTON_HEIGHT + 3 * SPACE_BETWEEN, BUTTON_WIDTH - 60, 16, ILI9341_BLACK);
    tft.setCursor(GAME_SIZE + 60, 2 * BUTTON_HEIGHT + 3 * SPACE_BETWEEN);
    tft.print(i);
}

uint16_t SolveBoardSetup() {
    /*
    PURPOSE
    Sets up the UI for the solving mode

    RETURNS
    numWords (uint16_t): The number of attainable words on the board
    */

    // Clears the Screen
    tft.fillRect(GAME_SIZE, 0, BUTTON_WIDTH, DISPLAY_HEIGHT, ILI9341_BLACK);
    ClearBoard();

    // Draws the 4 solving mode buttons
    tft.setTextSize(TEXT_SIZE);
    DrawButton(GAME_SIZE, SPACE_BETWEEN, "PREV"); // "PREV" button
    DrawButton(GAME_SIZE, 2 * SPACE_BETWEEN + BUTTON_HEIGHT, "NEXT"); // "NEXT" button
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "NEW"); // "NEW" button
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "SAME"); // "SAME" button

    // Changes text size and colpr
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    // Draws the word index indicator
    tft.setCursor(GAME_SIZE, 2 * BUTTON_HEIGHT + 3 * SPACE_BETWEEN);
    tft.print("WORD:"); 
    RedrawIndex(1);

    // Draws the "OF" indicator
    tft.setCursor(GAME_SIZE, 2 * BUTTON_HEIGHT + 5 * SPACE_BETWEEN);
    tft.print("OF:");

    // Finds the number of attainable words and prints it
    uint16_t numWords = ReadToNewLine().toInt();
    tft.setCursor(GAME_SIZE + 38, 2 * BUTTON_HEIGHT + 5 * SPACE_BETWEEN);
    tft.print(numWords);

    // Draws the possible points (PP) indicator
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(GAME_SIZE, 2 * BUTTON_HEIGHT + 8 * SPACE_BETWEEN);
    tft.print("PP:");

    // Finds the number of PP and prints it
    uint16_t possiblePoints = ReadToNewLine().toInt();
    tft.setCursor(GAME_SIZE + 38, 2 * BUTTON_HEIGHT + 8 * SPACE_BETWEEN);
    tft.print(possiblePoints);

    // Return the maximum number of words
    return numWords;
}

uint16_t SolvingButtons(TSPoint touch, uint16_t index, uint16_t numWords) {
    /*
    PURPOSE
    Checks to see if a button is pressed and executes the action of that button

    PARAMETERS
    touch (TSPoint): Holds the touch coordinates
    index (uint16_t): The current 'word index'
    numWords (uint16_t): The number of words possible to achieve 

    RETURNS
    index (uint16_t): The altered 'word index'
    */

    // If touch.x not in the button column, exit
    if (touch.x < GAME_SIZE)
        return index;

    // Check if "SAME" was pressed
    if (touch.y > GAME_SIZE - SPACE_BETWEEN) {
        Serial.println("0"); // 0 indicates new game, same board
        ClearBoard(); // Clears the board

        // Go back into the game, without generating new letters
        InGame(false);
    }
    // Checks if "NEW" was pressed
    else if (touch.y > GAME_SIZE - TILE_SIZE - SPACE_BETWEEN) {
        Serial.println("1"); // 0 indicates new game, same board
        ClearBoard(); // Clears the board
        
        // Go back into the game, without generating new letters
        InGame(true);
    }
    // Checks if "PREV" was pressed
    else if (touch.y < 1 * BUTTON_HEIGHT + 1 * SPACE_BETWEEN && index > 0) {
        index--; // Decremrent index
        RedrawIndex(index); // Redraw the index
        ClearBoard(); // Clears the board
        Serial.println("2"); // 2 indicates to get the previous word
        GetAndDrawWord(); // Draw the previous solved word
    }
    // Checks if "NEXT" was pressed
    else if (touch.y < 2 * BUTTON_HEIGHT + 2 * SPACE_BETWEEN && touch.y > SPACE_BETWEEN + BUTTON_HEIGHT
            && index < numWords) {
        index++; // Increment index
        RedrawIndex(index); // Redraw the index
        ClearBoard(); // Clears the board
        Serial.println("3"); // 3 indicates to get the next word
        GetAndDrawWord(); // Draw the next solved word
    }

    // Return the index
    return index;
}

void SolveBoard() {
    /*
    PURPOSE
    Display the solution for the board on the UI
    */

    // Gets the numWords while setting up the Solving UI
    uint16_t numWords = SolveBoardSetup();

    // Gets the first word and draws it on the board
    GetAndDrawWord();

    // index will hold the element of the solved words being looked at
    uint16_t index = 1;

    // Loop until a new game is started
    while (true) {
        // Get the touch coordinates
        TSPoint touch = processTouch();

        // Prevents innacuracies cause by holding down on the screen
        if (!pressCurr || pressPrev)
            continue;

        // Looks to see if a button is pressed and executes it
        index = SolvingButtons(touch, index, numWords);
    }
}

//**********************************END OF FUNCTION BLOCK*********************************//

//********************Functions that will be used at the end of the game********************//

void PostGameSetup() {
    /*
    PURPOSE
    Setup the UI for the post game mode
    */

    // Draws the 3 post game buttons
    tft.setTextSize(TEXT_SIZE);
    DrawButton(GAME_SIZE, SPACE_BETWEEN, "SOLVE"); // "SOLVE" button
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 3 * TILE_SIZE, "NEW"); // "NEW" button
    DrawButton(GAME_SIZE, 4 * SPACE_BETWEEN + 4 * TILE_SIZE, "SAME"); // "SAME" button

    // Sets text color to red and prints "GAME OVER!"
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    DrawWord("GAME OVER!");
}

void PostGameButtons(TSPoint touch) {
    /*
    PURPOSE
    Check to see if a button has been pressed and complete the action of a button pressed

    PARAMETERS
    touch (TSPoint): Holds the touch coordinates
    */

    // If touch isn't in button column, exit function
    if (touch.x < GAME_SIZE)
        return;

    // Checks to see if "SAME" pressed
    if (touch.y > GAME_SIZE - SPACE_BETWEEN) {
        // Sends a '0' to server to indicates new game, same board
        Serial.println("0");

        // Go back into the game, without generating new letters
        InGame(false);
    }
    // Checks to see if "NEW" pressed
    else if (touch.y > GAME_SIZE - TILE_SIZE - SPACE_BETWEEN) {
        // Sends a '1' to server to indicates new game, new board
        Serial.println("1");

        // Go back into the game, without generating new letters
        InGame(true);
    }
    // Checks to see if "SOLVE" pressed
    else if (touch.y < SPACE_BETWEEN + BUTTON_HEIGHT) {
        // Send a '2' to server to indicates entering solving mode
        Serial.println("2");

        // Solve the board
        SolveBoard();
    }
}

void PostGame() {
    /*
    PURPOSE
    Setup the UI for post game and wait for a button to be pressed
    */

    // Sets up the UI for post game
    PostGameSetup();

    // Loops until a button is pressed
    while (true) {
        // Get touch coordinates
        TSPoint touch = processTouch();

        // Prevents innacuracies caused by holding down on the screen
        if (!pressCurr || pressPrev)
            continue;

        // Checks to see if a button has been pressed
        PostGameButtons(touch);
    }
}

//**********************************END OF FUNCTION BLOCK**************************************//

//********************Functions that will be used during gameplay********************//

void GenerateLetters() {
    /*
    PURPOSE
    Randomly generate 16 letters based on the boggle dice
    */

    /* 'DICE' is a variable that holds the contents of all 16 boggle dice used in the original game,
    'DICE' contents won't change but the contents will be swapped so it can't be declared const */
    String DICE[16] = { "AEANEG", "AHSPCO", "ASPFFK", "OBJOAB", "IOTMUC", "RYVDEL",
                        "LREIXD", "EIUNES", "WNGEEH", "LNHNRZ", "TSTIYD", "OWTOAT",
                        "ERTTYL", "TOESSI", "TERWHV", "NUIHMQ" };

    // Variables that will hold pseudorandomly drawn letters
    int dieChosen;
    int letterChosen;

    // Randomly seeds the input based on the voltage at analog pin and time in milliseconds into the program
    randomSeed(analogRead(RAND_PIN) + millis());

    // Generates 16 random letters
    for (int i = 0; i < NUM_TILES; i++) {
        // Selects a random dice from the remaining set, selects a random letter from that dice 
        dieChosen = random(NUM_TILES - i);
        letterChosen = random(NUM_DICE_SIDES);

        // Append letter to string containing all board letters
        boardLetters += DICE[dieChosen][letterChosen];

        // Draws the letter into its respective spot on the board
        int col = i % 4; // Determines column on board
        int row = i / 4; // Determines row on board
        
        // Draws the letter in the respective tive
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
    /*
    PURPOSE
    Determines if a tile was prssed

    PARAMETERS
    touch (TSPoint): Touch coordinates from the most recent touch

    RETURNS
    tilePressed (uint8_t): The index of the tile that was pressed
    NUM_TILES: Indicates no tile was pressed
    */

    // Compute the tile that was pressed if touch was in the bounds of the game
    if (touch.x < GAME_SIZE && touch.y < GAME_SIZE) {
        // Finds the x and y coordinates of the press in relation to the board
        uint8_t x = constrain((touch.x) / (TILE_SIZE + SPACE_BETWEEN), 0, 3); // X coordinate
        uint8_t y = constrain((touch.y) / (TILE_SIZE + SPACE_BETWEEN), 0, 3); // Y coordinate

        // Index is calculated
        return (x + 4 * y); // Sum X and Y
    }
    
    // If touch not on boerd
    return NUM_TILES;

} 

uint32_t CheckTime(uint32_t start) {
    /*
    PURPOSE
    Checkes to see if the in game timer needs to be decremented then prints the new time if true

    PARAMETERS 
    start (uint32_t): The time since the last decrement

    RETURNS
    start (uint32_t): IF the timer not decremented
    currTime (uint32_t): IF the timer was incremented
    */

    // Declares a floating point variable to hold the current time 
    float currTime = millis() / MIL_PER_SEC; 

    // Checks to see if the timer should be decremented
    if (time - (currTime - start) != time) {
        // Find the number of digits in the time before we decrement
        uint8_t numDigits = String(time).length();
        
        // Decrement the timer
        time--;

        // If we lost a digit in the decrement, then fill in the previous number
        if (String(time).length() < numDigits) {
            tft.fillRect(GAME_SIZE + (BUTTON_WIDTH - (numDigits * 2 * 5 + (numDigits - 1) * 2)) / 2 , TILE_SIZE + 2 * SPACE_BETWEEN + 20, 
                     BUTTON_WIDTH, 2 * 8, ILI9341_BLACK); 
            // Reassign numDigits
            numDigits = String(time).length();
        }

        // Set the cursor to be centered in the button column and print the time
        tft.setCursor(GAME_SIZE + (BUTTON_WIDTH - (numDigits * 2 * 5 + (numDigits - 1) * 2)) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
        tft.print(time);
        
        // Return the new time
        return currTime;
    }

    // If time hasn't changed then return the same start time
    return start;
}

uint8_t TileValidity(uint8_t* visited, uint8_t tile ,uint8_t wLength) {
    /*
    PURPOSE
    Determine if a pressed tile is valid, A valid tile is:
    -Adjacent to the previous tile
    -Not already visited

    PARAMETERS
    visited (uint8_t*): A pointer to the array of all visited tiles
    tile (uint8_t): The index of the tile thay was pressed
    wLength (uint8_t): The current length of the word being built

    RETURNS
    tile (uint8_t): IF the tile passes all the tests
    NUM_TILES: IF the tile failed any test
    */

    // If this tile was the first pressed then it is guaranteed to be valid
    if (wLength == 0)
        return tile;

    // Checks if the tile already in the list of visited tiles
    for (int i = 0; i < wLength; i++)
        if (tile == visited[i])
            return NUM_TILES;

    // Gets the index for the previously visited tile 
    uint8_t prevTile = visited[wLength - 1];

    // Checks if the new tile is not adjacent 
    if (abs((tile % 4) - (prevTile % 4)) > 1 || abs((tile / 4) - (prevTile / 4)) > 1)
        return NUM_TILES;

    // All tests passed to return tile
    return tile;
}

void RedrawPoints() {
    /* 
    PURPOSE
    Redraws the amount of points that the player has
    */

    // Finds the number of digits
    uint8_t numDigits = String(points).length();

    // Centers the cursor in the button column and prints the point
    tft.setCursor(GAME_SIZE + (BUTTON_WIDTH - (numDigits * 2 * 5 + (numDigits - 1) * 2)) / 2, 2 * TILE_SIZE + 3 * SPACE_BETWEEN + 20);
    tft.print(points);
}

String Erase(String word, uint8_t* visited) {
    /*
    PURPOSE
    Removes the last letter of the built word

    PARAMETERS
    word (String): The word being sent to the server
    visited (uint8_t*): Pointer to the array of visited tiles

    RETUNRS
    word (String): The input word but without the very last letter
    */

    // Initializes a new string that will store all but the last letter of word
    String newWord = "";

    // Adds all but last letter
    for (int i = 0; i < word.length() - 1; i++)
        newWord += word[i];

    // Writes the trimmed word, unhighlights the tile, unconnects the tiles, removes from visited
    DrawWord(newWord);
    DrawTile(word[word.length() - 1], visited[word.length() - 1], ILI9341_WHITE);

    // If more than one tile is on the board then erase the connection between the previous tile
    if (word.length() > 1)
        ConnectTiles(visited[word.length() - 2], visited[word.length() - 1], ILI9341_BLACK);

    // Sets that tile to NUM_TILES in visited
    visited[word.length() - 1] = NUM_TILES;

    // Returns the shortened word
    return newWord;
}

String Enter(String word, uint8_t* visited) {
    /*
    PURPOSE
    Sends some word to the server then waits to see if it is valid or not

    PARAMETERS
    word (String): The word being sent to the server
    visited (uint8_t*): Pointer to the array of visited tiles

    RETUNRS
    "" (String): Will always return a blank string
    */

    // Declares a variable that will hold '0' or '1' depending on if the word is valid 
    char isWord;

    // Sends the word to the server
    Serial.println(word);

    // If the top most byte on the serial monitoe is not '0' or '1' then wait
    while (Serial.peek() != '0' && Serial.peek() != '1') {}

    uint8_t wordLength = word.length();

    // Clear the word and reset the visited tiles
    while (word != "") 
        word = Erase(word, &visited[0]);

    // Read in the top byte off the serial monitor
    isWord = Serial.read();
    
    // If '1' then adds to point count and redraws it
    if (isWord == '1') {
        points += constrain(wordLength - 2, 0, 6); // Adds points
        RedrawPoints(); // Redraws the points
        tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); // Sets text color to green 
        DrawWord("VALID WORD!"); // Writes "VALID WORD!"
    }
    else if (isWord == '0') {
        tft.setTextColor(ILI9341_RED, ILI9341_BLACK); // Sets text color to black
        DrawWord("INVALID!"); // Writes "INVALID!"
    }

    // Print an acknowledgement
    Serial.println("*");

    // Return the blank string
    return "";
}

String InGameButtons(TSPoint touch, String word, uint8_t* visited) {
    /*
    PURPOSE
    Determines if a button was pressed and executes its action

    PARAMETERS
    touch (TSPoint): Holds the touch coordinates
    word (String): Holds the current word build on the board
    visited (uint8_t*) A Pointer to the array of visited tiles

    RETURNS
    word (String): The modified word depending on button press
    */

    // Determines if somewhere on the buttons column was touched
    if (touch.x < GAME_SIZE)
        return word;

    // Determines if the enter button was pressed
    if (touch.y > GAME_SIZE - SPACE_BETWEEN && word.length() > 0)
        word = Enter(word, &visited[0]); // Send word to server and receive result

    // Determines if the erase button was pressed
    else if (touch.y > GAME_SIZE - TILE_SIZE - SPACE_BETWEEN && word.length() > 0) 
        word = Erase(word, &visited[0]); // Erase last letter of word

    // Determines if the solving button was pressed
    else if (touch.y < BUTTON_HEIGHT + SPACE_BETWEEN) {
        Serial.println("2"); // 2 incdicates entering solver mode
        ClearBoard(); // Clears the board
        SolveBoard(); // Enters the solving phase
    }

    // Delay by 200 milliseconds so it doesnt erase all at once
    delay(200);

    // Return the altered word
    return word;
}

void InGameSetup(bool newLetters) {
    /*
    PURPOSE
    Draws buttons on the right side and generates a new board if needed

    PARAMETERS
    newLetters (bool): Decides if a new board should be generated
    */

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
    
    // Writes the time indicator as well as the starting time
    uint8_t numDigits = String(time).length();
    tft.setCursor(GAME_SIZE + (BUTTON_COL_WIDTH - 45) / 2, TILE_SIZE + 2 * SPACE_BETWEEN);
    tft.print("TIME");
    tft.setCursor(GAME_SIZE + (BUTTON_WIDTH - (numDigits * 2 * 5 + (numDigits - 1) * 2)) / 2, TILE_SIZE + 2 * SPACE_BETWEEN + 20);
    tft.print(time);

    // Writes the points indicator as well as the starting points (0)
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
    /*
    PURPOSE
    This function will setup the InGame board, Processes button and tile presses, send words to the server 
    and anything else required from the InGame sequence

    PARAMETERS
    newLetters (bool): Determines if new letters need to be generated for the board
    */

    // Draws the in game UI
    InGameSetup(newLetters);

    // Initializes list to 16 invalid tile numbers, string containing word to be blank
    uint8_t visited[NUM_TILES] = {NUM_TILES}; // NUM_TILES is denoted as invalid
    String word = ""; // Initializes word to be blank

    // Starts a timer
    float startTime = millis() / MIL_PER_SEC;

    // While time remaining is more than 0
    while (time > 0) {
        // Checks if the timer needs to decrement by a second
        startTime = CheckTime(startTime);

        // Gets touch coordinates
        TSPoint touch = processTouch();

        // Ensures that the screen gets released after each press
        if (!pressCurr || pressPrev)
            continue;

        // Finds which tile (if any) was pressed, then checks if a button was pressed
        uint8_t tilePressed = TilePress(touch); // Gets tile number
        word = InGameButtons(touch, word, &visited[0]); // Checks if it was a random button

        // Checks to see if a valid tile was pressed
        tilePressed = TileValidity(&visited[0], tilePressed, word.length());
        
        // If a valid tile was pressed...
        if (tilePressed < NUM_TILES) {
            // Appends the letter to the word being built
            word += boardLetters[tilePressed];

            // Adds the tile index to array of visited tiles
            visited[word.length() - 1] = tilePressed;
            
            // Writes the word and highlights the tile pressed
            DrawWord(word);
            DrawTile(word[word.length() - 1], tilePressed, ILI9341_GREEN);

            // If this is not the first tile, connect to the previous tile
            if (word.length() > 1)
                ConnectTiles(visited[word.length() - 2], tilePressed, ILI9341_GREEN);
            
            // If this was the first tile pressed then box it in red
            else 
                BoxStartTile(tilePressed);
        }

        // Set previous state of press to the current state of press
        pressPrev = pressCurr;
    }
    // Send whatever word is left on the board
    word = Enter(word, &visited[0]);

    // Sound the alarm for the game being over
    tone(SOUND_PIN, 800, 1000);

    // Sends 1\n to the server to indicate the end of the game
    Serial.println("1");
    PostGame(); // Goes into the post game function
}

//**********************************************END OF FUNCTION BLOCK*************************************//


//********************Functions that will be used for setup********************//

void PreGameSetup() {
    /*
    PURPOSE
    Draws the UI for the PreGame setup
    */

    // Draws blank tiles (since letters haven't been determined yet)
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            tft.fillRect((col + 1) * SPACE_BETWEEN + col * TILE_SIZE, (row + 1) * SPACE_BETWEEN + row * TILE_SIZE,
                         TILE_SIZE, TILE_SIZE, ILI9341_WHITE);
        }
    }

    // Draws the white background for the "NEW GAME" button
    tft.fillRect(GAME_SIZE, SPACE_BETWEEN, BUTTON_WIDTH, DISPLAY_HEIGHT - 2 * SPACE_BETWEEN, ILI9341_WHITE);

    // Sets the text color to black on white
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);

    // Writes "NEW GAME" in the center of the button
    tft.setCursor(4 * TILE_SIZE + 5 * SPACE_BETWEEN + 25, DISPLAY_HEIGHT / 2 - CHAR_HEIGHT); // Sets cursor
    tft.print("NEW"); // Writes "NEW"
    tft.setCursor(4 * TILE_SIZE + 5 * SPACE_BETWEEN + 15, DISPLAY_HEIGHT / 2 + CHAR_HEIGHT / 2); // Sets cursor
    tft.print("GAME"); // Writes "GAME"
}

void PreGame() {
    /*
    PURPOSE
    Draws the UI before the start of a game and and waits for a button touch to enter the game
    */

    // Draws the UI for the PreGame
    PreGameSetup();

    // Waits until the 'NEW GAME' button is pressed
    while (true) {
        // Gets touch screen values 
        TSPoint touch = processTouch();

        // If "NEW GAME" is pressed then go to the in game sequence
        if (touch.x > GAME_SIZE)
            InGame(true); // Enters in game sequence, 'true' indicates to make a new board

    }
}

//********************************END OF FUNCTION BLOCK*********************************//

int main() {
    setup();
    PreGame();
    Serial.end();
    return 0;
}