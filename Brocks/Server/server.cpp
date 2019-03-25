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

// Initialize all the files that we will need later
const string boardFile = "TextFiles/board-graph.txt";
const string dictFile = "TextFiles/words.txt"; // from "http://www.gwicks.net/dictionaries.htm"
const string diceFile = "TextFiles/boggle-dice.txt";
 
#define NUM_SIDES_DIE 6 // Number of sides on a boggle die
#define NUM_TILES 16 // Number of tiles on a boggle board

using namespace std;

void InGame(bool newBoard);

// This wont be used for this file
string GenerateLetters(){
    // Randomizes the seed based on time 
    srand(time(NULL));

    // Initialize a filestream, and a variable that will hold dice letters
    ifstream allDice (diceFile);
    vector<string> dice;

    // Read in all the lines from the file
    string line;
    while(getline(allDice, line)) {
        // Line will be 6 letters in the form XXXXXX
        dice.push_back(line);
    }

    // Declares variable that will hold the number of dice left to be chosen 
    int numDice = NUM_TILES;

    // Declares variable that will store random numbers
    int dieChosen;
    int letterChosen;

    string letters;
    for (int i = 0; i < NUM_TILES; i++) {
        // Randomly choose from the remaining set of dice
        dieChosen = rand() % numDice;
        string dieLetters = dice[dieChosen];

        // Randomly choose a letter from the selected die
        letterChosen = rand() % NUM_SIDES_DIE;

        // Append that letter to the list of letters
        letters += dieLetters[letterChosen];

        // Erase the chosen die from the list of dice
        dice.erase(dice.begin() + dieChosen);
        numDice--;
    }
    // Close the file
    allDice.close();

    // Return the list of random letters
    return letters;
}

/*************************Functions For Solving Mode**************************/
void solveTile(string currWord, vector<int> tilesVisited, Digraph board) {
    // Recurse 1 layer down if the currWord is not a subtring of any dictionary word
    if (!dict.searchPrefix(currWord))
        return;
    
    // Remove all of the edges that lead to the current tile (So that we can't go back to it)
    for (auto it = board.neighbours(tilesVisited.back()); it != board.endIterator(tilesVisited.back()); it++) {
        board.removeEdge(*it, tilesVisited.back());
    }

    // Iterate through all of the tiles neighbours
    for (auto it = board.neighbours(tilesVisited.back()); it != board.endIterator(tilesVisited.back()); it++) {
        // Append the tile visited to the vector
        tilesVisited.push_back(*it);
        currWord.push_back(letters[*it]);
        solveTile(currWord, tilesVisited, board);

        // Remove the letter appended
        currWord.pop_back();
        tilesVisited.pop_back();
    }

    // When we've iterated through all of a tiles neighbours, check to see if the tile is in dictionary
    if (dict.searchWord(currWord))
        boggleWords.push_back(pair<string, vector<int>>(currWord, tilesVisited));

    // End of function reached so it will recurse 1 layer out
}

void IterateBoard(Digraph board) {
    for (int i = 0; i < NUM_TILES; i++) {
        // Initialize the string to be blank
        string firstLetter = ""; 
        vector<int> tiles = {};

        // Append the tiles letter to the string
        firstLetter.push_back(letters[i]);
        tiles.push_back(i);

        // Find all words that originate at that block
        solveTile(firstLetter, tiles, board);
    }
}

void QuickSortLength(int start, int end) {
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
    // String that will store unique words
    vector<pair<string, vector<int>>> nonRepeats;
    
    // 'prev' tracks the index that letters of some length start at
    int prev = 0; 

    for (int i = 1; i < boggleWords.size(); i++) {
        // If current element string is shorter than previous sort the previous alphabetically
        if (boggleWords[i].first.length() < boggleWords[i - 1].first.length()) {
            QuickSortAlpha(prev, i - 1); // Sort the array of letters of some length
            prev = i; // Reassign Prev
        }
    }
    // The last range of words has to be sorted because the loop will end before it sorts them
    QuickSortAlpha(prev, boggleWords.size() - 1);

    // Iterate through the now alphabetially by length sorted list
    for (int i = 1; i < boggleWords.size(); i++) {
        // Since its sorted, identical elements will be right next to eachother
        // So if two consecutive elements are identical leave out the second one
        if (boggleWords[i].first != boggleWords[i - 1].first)
            nonRepeats.push_back({boggleWords[i].first, boggleWords[i].second});
    }

    // Returns the newly assigned list of words and their respective paths on the board 
    return nonRepeats;
}

int PossiblePoints() {
    // Initialize a point counter to 0
    int totalPoints = 0;

    // Iterate through all the possible words
    for (auto i : boggleWords) {
        if (i.first.length() < 9)
            totalPoints += i.first.length() - 2;

        // In the real game boggle, points top out at 6 for words 8 letters or longer
        else 
            totalPoints = 6;
    }

    return totalPoints;
}

void SendWord() {
    // Send the word until an acknowledgement is read
    do {
        Serial.writeline(boggleWords[0].first);
        Serial.writeline("\n");
    } while (Serial.readline() != "*\n");

    // Send all tiles that make up the path, wait for an acknowledgement each time
    for (int i = 0; i < boggleWords[0].first.length(); i++) {
       do {
            Serial.writeline(to_string(boggleWords[0].second[i]));
            Serial.writeline("\n");
        } while (Serial.readline() != "*\n");
    }
}

void SendData(int totalPoints, int numWords) {
    cout << "HERE" << endl;
    do {
        Serial.writeline(to_string(numWords));
        Serial.writeline("\n");
    } while (Serial.readline() != "*\n");

    do {
    Serial.writeline(to_string(totalPoints));
    Serial.writeline("\n");
    } while (Serial.readline() != "*\n");

    SendWord();
    while(true) {};
}

void Solving() {
    IterateBoard(board);

    QuickSortLength(0, boggleWords.size() - 1);

    boggleWords = EliminateRepeats();
    int totalPoints = PossiblePoints();
    cout << "LIST OF ALL POSSIBLE WORDS\n";
    for (auto i : boggleWords) {
        cout << i.first <<", ";
        for (auto j : i.second)
            cout << j << " ";
    cout << endl;
    }
    
    cout << "Number of Attainable words: " << boggleWords.size() << endl << "Number of Attainable Points: " << totalPoints <<endl;
    SendData(totalPoints, boggleWords.size());
}
/***************************END OF FUNCTION BLOCK*****************************/



/*************************Functions For Post Game Mode**************************/
string WaitForMode() {
    string mode = "";
    while (mode == "")
        mode = Serial.readline();

    return mode;
}

void EndGame() {
    string mode = WaitForMode();

    wordsEntered.clear();
    if (mode == "0\n")
        InGame(false);
    else if (mode == "1\n") 
        InGame(true);
    else if (mode == "2\n")
        Solving();
}

/***************************END OF FUNCTION BLOCK*****************************/



/*************************Functions For In Game Mode**************************/
void PrintBoard() {
    cout << "Board looks like:\n";
    //Prints out the word board
    for (int i = 0; i < 16; i++) {
        cout << letters[i] << " ";
        if (i % 4 == 3) 
            cout << endl;
    }
    cout << endl;
}

void CheckWordReceived(string line) {
    cout << "Checking to see if '"<< line << "' is a valid word"<< endl;
    // If a 2\n is received, this indicates that SOLVE was pressed
    if (line == "2")
        Solving();

    if (dict.searchWord(line) && wordsEntered.find(line) == wordsEntered.end()) {
        Serial.writeline("1");
        wordsEntered.insert(line);
        cout << "VALID" << endl;
    }
    else {
        Serial.writeline("0");
        cout << "INVALID" << endl;
    }
    cout << "\nWaiting for a word...\n\n";
}

void WaitForWords() {
    // Declare a variable to hold line
    string line = "";
    cout << "Waiting for first word...\n\n";
    // If not told to move to next phase then keep looping
    while (line != "NEXT PHASE\n") {
        // Read in a line off of the serial monitor
        line = Serial.readline();

        // If the line is not blank...
        if (line != "" && line != "NEXT PHASE\n") {
            // Get rid of the \n that will be attached and check if its a word
            line = line.substr(0, line.length() - 1);
            CheckWordReceived(line);
        }
    }
}

void GetBoardLetters() {
    cout << "Waiting For Board Letters..." << endl;
    // Gets the board letters from the arduino
    char letter;
    letters = "";

    while (letters == "") 
        letters = Serial.readline();
}

void InGame(bool newBoard){
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
Trie MakeTrie() {
    // Initialize a filestream
    ifstream allWords(dictFile);

    //Initialize an object of the Trie class
    Trie dictionary;

    // Read in all the lines from the file
    string word;
    while (getline(allWords, word)) {
        // Adds the word to the Trie object
        dictionary.insert(word);
    }
    // Close the file
    allWords.close();

    // Returns the Trie object
    return dictionary;
}

Digraph createBoard() {
    // Initialize an object of the Digraph class
    Digraph board;

    // Add a vertex at all 16 Blocks
    for (int i = 0; i < NUM_TILES; i++)
        board.addVertex(i);


    for (int i = 0; i < NUM_TILES; i++) {
        int x1 = i % 4;
        int y1 = i / 4;

        for (int j = 0; j < NUM_TILES; j++) {
            if (j == i) 
                continue;

            int x2 = j % 4;
            int y2 = j / 4;

            if (abs(x1 - x2) <= 1 && abs(y1 - y2) <= 1)
                board.addEdge(i, j);
        }
    }

    // Return the board that was created
    return board;
}

void PreGameSetup() {
    // Create an object of the Trie class, this object will holds dictionary information
    dict = MakeTrie();

    // Create an object of the Digraph class, this object is a graph of a boggle board
    board = createBoard();
}

void PreGame() {
    // Sets up the server
    PreGameSetup();

    // Go to the in game once letters are received
    InGame(true);
}
/***************************END OF FUNCTION BLOCK*****************************/




int main() {  
    PreGame();
    return 0;
}
