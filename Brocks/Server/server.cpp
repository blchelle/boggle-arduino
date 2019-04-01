/*
Names: Brock Chelle, Benjamin Wagg
IDs: 1533398, 1531566
CCID: bchelle, bwagg
Course: CMPUT 275
Term: Winter 2019
Final Project: Boggle Solver (Part 2)
*/

#include <iostream> // Gives access to stdin, stdout

#include <string> // Gives access to strings
#include <vector> // Gives access to vecor data structures
#include <unordered_set> // Gives access to unordered_set

#include <ctime> // Gives access to random number generation
#include <fstream> // Gives access to files

#include "digraph.h" // Gives access to the Digraph class
#include "trie.h" // Gives access to the Trie class
#include "serialport.h" // Gives access to the SerialPort class

vector<pair<string, vector<int>>> boggleWords; // Stores all words attainable on a given boggle board
unordered_set<string> wordsEntered;
Digraph board;
Trie dict; // Stores a given dictionary in a Trie data structure
string letters; // Stores all the letters on a randomly generated boggle board

// Initialize the Serial Communication
SerialPort Serial("/dev/ttyACM0");

// Initialize the file that contains our dictionary
const string dictFile = "TextFiles/words.txt";

#define NUM_SIDES_DIE 6 // Number of sides on a boggle die
#define NUM_TILES 16 // Number of tiles on a boggle board

using namespace std;

// InGame is predeclared becuase it is called from functions above it
void InGame(bool newBoard);

/*************************Functions For Solving Mode**************************/

void SolveTile(string currWord, vector<int> tilesVisited, Digraph board) {
    /*
    PURPOSE
    Finds all the words attainable from a given start tile along with their paths

    PARAMETERS
    currWord (String): The word built up by the path travelled so far
    tilesVisited (vector<int>): A vector holding all the tiles visited so far
    
    board (Digraph): Hold the graph of the board 
    (Not using the global version since edges will be removed as the function recurses deeper)
    */

    // Recurses 1 layer out if the currWord is not a substring of any dictionary word
    if (!dict.searchWord(currWord, true))
        return;
    
    // Removes all of the edges that lead to the current tile (So that we can't go back to it)
    for (auto it = board.neighbours(tilesVisited.back()); it != board.endIterator(tilesVisited.back()); it++) {
        board.removeEdge(*it, tilesVisited.back());
    }

    // Iterates through all of the tiles neighbours
    for (auto it = board.neighbours(tilesVisited.back()); it != board.endIterator(tilesVisited.back()); it++) {

        // Appends the tile visited to the vector
        tilesVisited.push_back(*it);
        currWord.push_back(letters[*it]);
        SolveTile(currWord, tilesVisited, board);

        // Removes the letter appended
        currWord.pop_back();
        tilesVisited.pop_back();
    }

    // When we've iterated through all of a tiles neighbours, check to see if the tile is in dictionary
    if (dict.searchWord(currWord, false))
        boggleWords.push_back(pair<string, vector<int>>(currWord, tilesVisited));

    // End of function reached so it will recurse 1 layer out
}

void SolveBoard() {
    /*
    PURPOSE
    Execute the solve tile function from all 16 staring blocks on the board
    */

    // Iterate through all the starting tiles
    for (int i = 0; i < NUM_TILES; i++) {
        // Initialize the string to be blank
        string firstLetter = ""; 
        vector<int> tiles = {};

        // Append the tiles letter to the string
        firstLetter.push_back(letters[i]);
        tiles.push_back(i);

        // Find all words that originate at that block
        SolveTile(firstLetter, tiles, board);
    }
}

void QuickSortLength(int start, int end) {
    /*
    PURPOSE
    Sorts a range of words by length

    PARAMETERS
    start (int): The index of the array to start sorting from
    end (int): The index of the array to end at
    */

    // Let the partition value to the length of the first word of boggleWords
    int partitionVal = boggleWords[start].first.length(); 

    // Creates temporary variables for the start and end
    int i = start;
    int j = end;

    // Performs the partition part of Quick Sort
    while (j >= i) {
        // Decrement j until the value held at boggleWords[j] is less than the value at boggleWords[partition]
        while (boggleWords[j].first.length() < partitionVal)
          j--;
        // Increment i until the value held at boggleWords[i] is greater or equal to the value at boggleWords[partition]
        while (boggleWords[i].first.length() > partitionVal)
          i++;
        // If i < j, swap boggleWords[i], boggleWords[j]
        if (j >= i) { 
          //Store boggleWords[i] in a temporary variable so that we can swap 
          pair<string, vector<int>> temp = boggleWords[i];
          boggleWords[i] = boggleWords[j];
          boggleWords[j] = temp;
          j--;
          i++;
        }
    }

    // If start is less than j, run quicksort on a shorter array froma boggleWords[start] to boggleWords[j]
    if (start < j)
        QuickSortLength(start, j);
    // If i is less than n, run quicksort on a shorter array froma boggleWords[i] to boggleWords[end]
    if (i < end)
        QuickSortLength(i, end);
}

void QuickSortAlpha(int start, int end) {
    /*
    PURPOSE
    Sorts a range of words alphabetically

    PARAMETERS
    start (int): The index of the array to start sorting from
    end (int): The index of the array to end at
    */

    // Sets the partition to the string at the start index
    string partitionVal = boggleWords[start].first;

    // Creates temporary variables for the start and end
    int i = start;
    int j = end;

    // Performs the partition part of Quick Sort
    while (j >= i) {
        // Decrement j until the value held at boggleWords[j] is less than the value at boggleWords[partition]
        while (boggleWords[j].first.compare(partitionVal) > 0)
            j--;
        // Increment i until the value held at boggleWords[i] is greater or equal to the value at boggleWords[partition]
        while (boggleWords[i].first.compare(partitionVal) < 0) 
            i++;
        // If i < j, swap boggleWords[i], boggleWords[j]
        if (j >= i) { 
          //Store boggleWords[i] in a temporary variable so that we can swap 
          pair<string, vector<int>> temp = boggleWords[i];
          boggleWords[i] = boggleWords[j];
          boggleWords[j] = temp;
          j--;
          i++;
        }
    }

    // If start is less than j, run quicksort on a shorter array froma boggleWords[start] to boggleWords[j]
    if (start < j)
        QuickSortAlpha(start, j);
    // If i is less than n, run quicksort on a shorter array froma boggleWords[i] to boggleWords[end]
    if (i < end)
        QuickSortAlpha(i, end);
}

vector<pair<string, vector<int>>> EliminateRepeats() {
    /*
    PURPOSE
    Eliminate repeated words on the list of solved words

    RETURNS
    nonRepeats (vector<pair<string, vector<int>>>): 
    Returns the list of solved words where each word appears only once
    */

    // Variable that will store each word of the solved word only once
    vector<pair<string, vector<int>>> nonRepeats;
    
    // 'prev' tracks the index that letters of some length start at
    int prev = 0; 

    // Iterate through the list of words
    for (int i = 1; i < boggleWords.size(); i++) {
        // If current element string is shorter than previous sort the previous alphabetically
        if (boggleWords[i].first.length() < boggleWords[i - 1].first.length()) {
            QuickSortAlpha(prev, i - 1); // Sort the array of letters of some length
            prev = i; // Reassign prev
        }
    }
    // The last range of words has to be sorted because the loop will end before it sorts them
    QuickSortAlpha(prev, boggleWords.size() - 1);

    // Iterate through the list now sorted alphabetially by length 
    for (int i = 0; i < boggleWords.size(); i++) {
        // Since its sorted, identical elements will be right next to eachother
        // So if two consecutive elements are identical leave out the second one

        if (boggleWords[i].first != boggleWords[i + 1].first || i == boggleWords.size() - 1)
            nonRepeats.push_back({boggleWords[i].first, boggleWords[i].second});
    }

    // Returns the newly assigned list of words and their respective paths on the board 
    return nonRepeats;
}

int PossiblePoints() {
    /*
    PURPOSE
    Calculate the number of points given if all words were found

    RETURNS
    totalPoints (int): The amount of points found 

    BOGGLE SCORING SYSTEM
    Points for a word are equal to the words length - 2. 
    However the max points for any word are 6, so words 9 letters or longer are 6 points
    */

    // Initialize a point counter to 0
    int totalPoints = 0;

    // Iterate through all the possible words
    for (auto i : boggleWords) {
        // Adds to points list based on the boggle scoring system
        if (i.first.length() < 9)
            totalPoints += i.first.length() - 2;

        // In the real game boggle, points top out at 6 for words 8 letters or longer
        else 
            totalPoints += 6;
    }

    // Return the amount of points found
    return totalPoints;
}

void SendWord(int index) {
    /*
    PURPOSE
    Sends a word from the possible words list along with the path on the 
    boggle board required to reach it
    
    PARAMETERS
    index (int): The index of the possible word list to send
    */

    // Sends the length of the word until the client sends an acknowledgement
    do {
        Serial.writeline(to_string(boggleWords[index].first.length()));
        Serial.writeline("\n");
    } while (Serial.readline() != "*\n");

    // Send all tiles that make up the path, waits for an acknowledgement each time
    for (int i = 0; i < boggleWords[index].first.length(); i++) {
       // Sends a tile index until the client sends an acknowledgement
       do {
            Serial.writeline(to_string(boggleWords[index].second[i]));
            Serial.writeline("\n");
        } while (Serial.readline() != "*\n");
    }

    // Indicates which word was sent to the client 
    cout << boggleWords[index].first << " Sent to serial monitor\n\n";
}

void SendData(int totalPoints, int numWords) {
    /*
    PURPOSE
    Send Possible points and Number of words to the client

    PARAMETERS
    totalPoints (int): The number of attainable points on the board
    numWords (int): The number of attainable words on the board
    */
    
    // Send the number of attainable words until the client acknowledges
    do {
        Serial.writeline(to_string(numWords));
        Serial.writeline("\n");
    } while (Serial.readline() != "*\n");

    // Send the number of attainable points until the client acknowledges
    do {
    Serial.writeline(to_string(totalPoints));
    Serial.writeline("\n");
    } while (Serial.readline() != "*\n");

    // Send the first word along with its mapping to the client
    SendWord(0);
}

void GetWordData() {
    /*
    PURPOSE
    List all the found words as well as the paths on the board needed to get them,
    Also finds the number of points attainable on the board. Sends data to the client
    */

    // Declares a variable that will hold the number of attainable points
    int totalPoints = PossiblePoints();

    // Indicates all the words are about to be listed
    cout << "LIST OF ALL POSSIBLE WORDS\n";

    // Iterate through the list of words found
    for (auto i : boggleWords) {
        // Print the word
        cout << i.first <<", ";

        // Print the mapping on the boggle board to make it
        for (auto j : i.second)
            cout << "("<< j % 4 << ", " << j / 4 << ") ";

    // Output a newline for formatting
    cout << endl;
    }
    // Print out the number of attainable words and number of attainable points
    cout << "Number of Attainable words: " << boggleWords.size() << endl;
    cout << "Number of Attainable Points: " << totalPoints << "\n\n";

    // Send Possible Points and number of words to the serial monitor
    SendData(totalPoints, boggleWords.size());
}

int WaitForInput(int index) { 
    /*
    PURPOSE
    Waits for the client so send a command, then executes that command

    PARAMETERS
    index (int): Holds the current index of the solved words list

    RETURNS
    index (int): The Index of the solved words list (Only changed if NEXT or PREV was pressed) 
    */

    // Declares a variable thst will hold the input
    string line = "";

    // Indicates that the server is waiting for input
    cout << "Waiting For Button Press...\n";

    // Loops until a wordnis read from the serial monitor
    while (line == "")
        line = Serial.readline();

    // Stores all but the \n character at the end
    line = line.substr(0, line.length() - 1);

    // "0" indicates to play again but on the same board
    if (line == "0") {
        cout << "NEW GAME (SAME BOARD)\n";
        InGame(false);
    }
    // "1" indicates to play again on a new board
    else if (line == "1") {
        cout << "NEW GAME (NEW BOARD)\n";
        InGame(true);
    }
    // "2" indicates to send the previous word in the solved words list
    else if (line == "2") {
        index--; // Decrement index
        cout << "SENDING PREVIOUS SOLVED WORD\n";
        SendWord(index); // Send word
    }
    // "3" indicates to send the next word in the solved words list
    else if (line == "3") {
        index++; // Increment index
        cout << "SENDING NEXT SOLVED WORD\n";
        SendWord(index); // Send word
    }

    // Returns the index (Only changed if NEXT or PREV was pressed)
    return index;
}

void Solving() {
    /*
    PURPOSE
    Solves the board by finding all possible word on the board and then sorts them 
    by length and alphabetically
    */

    // Solves the boggle board
    SolveBoard();

    // Sorts the board by length (Most points)
    QuickSortLength(0, boggleWords.size() - 1);

    // Eliminate words that appear more than once 
    boggleWords = EliminateRepeats();

    // Gets data about all the words (Num Words and Possible Poins), then sends to client
    GetWordData();

    // Set the index to 0, index determines which word to send to the client
    int index = 0;

    // Loops until the client says to play another game
    while (true) 
        // Waits for input from the client
        index = WaitForInput(index);
}

/***************************END OF FUNCTION BLOCK*****************************/


/*************************Functions For Post Game Mode**************************/

string WaitForMode() {
    /*
    PURPOSE
    Waits for the client to send the mode that the game will move to next
    */

    // Initilizes mode to be blank
    string mode = "";

    // Keeps reading until a mode is received
    while (mode == "")
        mode = Serial.readline();

    // Returns all but the \n character appended to the end
    return mode.substr(0, mode.length() - 1);
}

void EndGame() {
    /*
    PURPOSE
    Wait for a button to be pressed on the client and go to the mode pressed
    */

    // Looks for a mode
    string mode = WaitForMode();

    // Clears the words entered for next game
    wordsEntered.clear();

    // If mode is "0", this indicates to play again on the same board
    if (mode == "0")
        InGame(false);
    // If mode is "1", this indicates to play again on a new board
    else if (mode == "1") 
        InGame(true);
    // If mode is "2",this indicates to enter solving mode
    else if (mode == "2")
        Solving();
}

/***************************END OF FUNCTION BLOCK*****************************/


/*************************Functions For In Game Mode**************************/

void PrintBoard() {
    /*
    PURPOSE
    Outputs the board on the terminal
    */

    // Indicates that the board will follow this message
    cout << "Board looks like:\n";

    //Prints out the word board
    for (int i = 0; i < NUM_TILES; i++) {
        // Output a letter followed by a space
        cout << letters[i] << " ";
        
        // Print new line if at the end of a row
        if (i % 4 == 3) 
            cout << endl;
    }
    // Print an extra newline for formatting
    cout << endl;
}

void CheckWordReceived(string line) {
    /*
    PURPOSE
    Determine if the word sent is in the dictionary

    PARAMETERS
    line (string): The word sent by the client
    */

    // Indicates the word we are searching for
    cout << "Checking to see if '"<< line << "' is a valid word"<< endl;

    do {
        // If word is in dictionary, then send 1 and indicate that its valid
        if (dict.searchWord(line, false) && wordsEntered.find(line) == wordsEntered.end()) {
            Serial.writeline("1");
            wordsEntered.insert(line); // Adds to word entered
            cout << "VALID" << endl;
        }
        // If word is not in dictionary, then send 0 and indicate that it is invalid
        else {
            Serial.writeline("0");
            cout << "INVALID" << endl;
        }
      // Loop this process until an acknowledgement is read from the serial monitor
    } while (Serial.readline() != "*\n");

    // Indicates that the server is waiting for the next word
    cout << "\nWaiting for a word...\n";
}

void WaitForWords() {
    /*
    PURPOSE
    Wait for a word to be sent from the client and then confirm or deny the word
    */

    // Declare a variable to hold line
    string line = "";

    // Indicate that the server is waiting for a word
    cout << "Waiting for first word...\n";

    // If not told to move to next phase then keep looping ('1\n' indicates game over)
    while (line != "1\n") {
        // Reads in a line from the serial monitor
        line = Serial.readline();

        // If the line is not blank and its not the end of the game
        if (line != "" && line != "1\n") {
            // Gets rid of the \n that will be attached and check if its a word
            line = line.substr(0, line.length() - 1);

            // If a 2 is received, this indicates that SOLVE was pressed
            if (line == "2") {
                // Indicates that solving mode is being entered, enter solving mode
                cout << "Entering Solving Mode\n";

                // Clear the words entered for next game
                wordsEntered.clear();
                Solving();
            }

            // Check to see if the word sent is valid
            CheckWordReceived(line);
        }
    }
    // Indicates that the game has ended
    cout << "TIME UP!\n\n";
}

void GetBoardLetters() {
    /*
    PURPOSE
    Receive board letters from the client
    */

    // Indicates that the server is waiting for board letters
    cout << "Waiting For Board Letters...\n" << endl;

    // Gets the board letters from the arduino
    char letter;
    letters = "";

    // If there is nothing on the serial monitor then keep reading
    while (letters == "") 
        letters = Serial.readline();
}

void InGame(bool newBoard){
    /*
    PURPOSE
    Get the board letters from the client and confirm or deny inputted words

    PARAMETERS
    newBoard (bool): Determines if a new board should be received
    */

    // Clear all the words found
    boggleWords.clear();

    if (newBoard)
        // Waits for board letters to come from the arduino
        GetBoardLetters();

    // Print out the created board
    PrintBoard();

    // Wait for words to be sent to the server
    WaitForWords();

    // Go to end game when the time runs out 
    EndGame();
}

/***************************END OF FUNCTION BLOCK*****************************/


/*************************Functions For Pre Game Mode**************************/

void MakeTrie() {
    /*
    PURPOSE
    Create an object of the Trie class for quick lookups
    */

    // Initialize a filestream
    ifstream allWords(dictFile);

    // Read in all the lines from the file
    string word;
    while (getline(allWords, word)) {
        // Adds the word to the Trie object
        dict.insert(word);
    }
    // Close the file
    allWords.close();
}

void CreateBoard() {
    /*
    PURPOSE
    Create an object of the digraph class that connects all adjacent and diagonal edges on the board
    */

    // Variable that will hold all the possible neighbours
    vector<int> neighbours;

    // Add a vertex at all 16 Blocks
    for (int i = 0; i < NUM_TILES; i++)
        board.addVertex(i);

    // Iterate thorugh all the tiles and determine which neighbours are valid
    for (int i = 0; i < NUM_TILES; i++) {
        // Initialize all the possible neighbours
        neighbours = {i - 5, i - 4, i - 3, 
                      i - 1,        i + 1,
                      i + 3, i + 4, i + 5};

        // Iterate through all the possible neighbours.
        for (auto n : neighbours) 
            // Neighbours must be...
            // a) On the board
            // b) Be within 1 for both x & y coordinates
            if (abs(i % 4 - n % 4) <= 1 && abs(i / 4 - n / 4) <= 1 && 
                n % 4 >= 0 && n / 4 >= 0 && n % 4 <= 3 && n / 4 <= 3)
                
                // Adds the edge to the board
                board.addEdge(i, n);
    }
}

void PreGame() {
    /*
    PURPOSE
    Setup objects of the tree and Digraph classes and enter the InGame mode
    */

    // Create an object of the Trie class, this object will holds dictionary information
    MakeTrie();

    // Create an object of the Digraph class, this object is a graph of a boggle board
    CreateBoard();

    // Go to the in game where letters will be received
    InGame(true);
}

/***************************END OF FUNCTION BLOCK*****************************/


int main() { 
    // Enter the pregame/setup phase 
    PreGame();
    return 0;
}
