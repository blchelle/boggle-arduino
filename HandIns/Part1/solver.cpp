/*
Names: Brock Chelle, Benjamin Wagg
IDs: 1533398, 1531566
CCID: bchelle, bwagg
Course: CMPUT 275
Term: Winter 2019
Final Project: Boggle Solver (Part 1)
*/

#include <iostream> // Gives access to stdin, stdout

#include <string> // Gives access to strings
#include <vector> // Gives access to vecor data structures
#include <unordered_set> // Gives access to unordered_set

#include <ctime> // Gives access to random number generation
#include <time.h> // Gives access to timing the program
#include <fstream> // Gives access to files
#include <ctype.h> // Gives access to the toupper function

#include "digraph.h" // Gives access to the Digraph class
#include "trie.h" // Gives access to the Trie class

vector<pair<string, vector<int>>> boggleWords; // Stores all words attainable on a given boggle board
unordered_set<string> wordsEntered;
Digraph board;
Trie dict; // Stores a given dictionary in a Trie data structure
string letters = ""; // Stores all the letters on a randomly generated boggle board

// Initialize all the files that we will need later
const string dictFile = "TextFiles/words.txt"; // Contains the dictionary used
const string diceFile = "TextFiles/boggle-dice.txt"; // Contains all the dice information
 
#define NUM_SIDES_DIE 6 // Number of sides on a boggle die
#define NUM_DICE 16 // Number of dice on a board

using namespace std;

void SolveTile(string currWord, vector<int> tilesVisited, Digraph board) {
    /*
    PURPOSE
    Finds all the words attainable from a given start tile along with their paths

    PARAMETERS
    currWord (String): The word built up by the path travelled so far
    tilesVisited (vector<int>): A vector holding all the tiles visited so far
    
    board (Digraph): Hold the graph of the board 
    (Not using the global version since edges will be removed as the function recurses deeper)

    COMPLEXITY
    O(8^(n^2))
    */

    // Recurse 1 layer down if the currWord is not a subtring of any dictionary word
    if (!dict.searchWord(currWord, true))
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
        SolveTile(currWord, tilesVisited, board);

        // Remove the letter appended
        currWord.pop_back();
        tilesVisited.pop_back();
    }

    // When we've iterated through all of a tiles neighbours, check to see if the tile is in dictionary
    if (dict.searchWord(currWord, false))
        boggleWords.push_back(pair<string, vector<int>>(currWord, tilesVisited));

    // End of function reached so it will recurse 1 layer out
}

void SolveBoard(int boardSize) {
    /*
    PURPOSE
    Execute the solve tile function from all 16 staring blocks on the board

    PARAMETERS
    boardSize (int): The number of tiles on the board

    COMPLEXITY 
    O(boardSize)
    */

    // Iterate through all of the tiles
    for (int i = 0; i < boardSize; i++) {
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

    COMPLEXITY
    O(n*log(n)), Where n is the number of words found
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

    COMPLEXITY
    O(n*log(n)), Where n is the number of words found
    */

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

    COMPLEXITY
    O(n*log(n) + n)
    */

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
    Calculates the maximum number of points achievable

    RETURNS
    totalPointsn (int): The calculated number of attainable points

    COMPLEXITY
    O(n), Where n is the number of words found
    */

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

void PrintBoard(int x, int y) {
    /*
    PURPOSE
    Outputs the board on the terminal

    PARAMETERS
    x (int): The number of columns
    y (int): The number of rows

    COMPLEXITY
    O(x*y)
    */

    cout << "Board looks like:\n";
    //Prints out the word board
    for (int i = 0; i < x * y; i++) {
        cout << letters[i] << " ";
        if (i % x == x - 1) 
            cout << endl;
    }
    cout << endl;
}

void GenerateLetters(int numLetters) {
    /*
    PURPOSE
    Randomly generate board letters

    COMPLEXITY
    O(numLetters)
    */

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

    // Declares variable that will store random numbers
    int dieChosen;
    int letterChosen;

    int count = 0;
    while (count < numLetters) {
        for (int i = 0; i < NUM_DICE; i++) {
            // Randomly choose from the remaining set of dice
            dieChosen = rand() % (NUM_DICE - i);
            string dieLetters = dice[dieChosen];

            // Randomly choose a letter from the selected die
            letterChosen = rand() % NUM_SIDES_DIE;

            // Append that letter to the list of letters
            letters += dieLetters[letterChosen];

            // Swap the index to the end so it cant be selected again
            string temp = dice[dieChosen];
            dice[dieChosen] = dice[NUM_DICE - (1 + i)];
            dice[NUM_DICE - (1 + i)] = temp;

            count++;
            if (count == numLetters) 
                break;
        }
    }
    // Close the file
    allDice.close();
}

void MakeTrie() {
    /*
    PURPOSE
    Makes the trie object to hold the dictionary

    COMPLEXITY
    O(d), Where d is the number of words in the dictionary
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

void CreateBoard(int x, int y) {
    /*
    PURPOSE
    Create the boject of the digraph class

    PARAMETERS
    x (int): The number of columns
    y (int): The number of rows

    COMPLEXITY
    O(x*y)
    */

    vector<int> neighbours;

    // Add a vertex at all tiles
    for (int i = 0; i < x * y; i++)
        board.addVertex(i);

    // Edge Case, if only 1 column then... 
    if (x == 1) {
        for (int i = 1; i < y - 1; i++) {
            board.addEdge(i, i + 1);
            board.addEdge(i + 1, i);
        }
        return;
    }
    // Edge case, if only 1 row then...
    else if (y == 1) {
        for (int i = 1; i < x - 1; i++) {
            board.addEdge(i, i + 1);
            board.addEdge(i + 1, i);
        }
        return;
    }

    // Iterate through all the tiles
    for (int i = 0; i < x*y; i++) {
        if (x != 2) {
            // Initialize all the possible neighbours
            neighbours = {i - x - 1, i - x, i - x + 1, 
                          i - 1,                i + 1,
                          i + x - 1, i + x, i + x + 1};
        }

        // Edge Case, neighbours differ if only 2 columns
        if (x == 2) {
            neighbours = {i - 3, i - 2, i - 1,
                          i + 1, i + 2, i + 3};
        }

        // Iterate through all the possible neighbours.
        for (auto n : neighbours) 
            // Neighbours must be...
            // a) On the board
            // b) Be within 1 for both x & y coordinates
            if (abs(i % x - n % x) <= 1 && abs(i / x - n / x) <= 1 && 
                n % x >= 0 && n / x >= 0 && n % x <= x - 1 && n / x <= y - 1
                && n >= 0 && n < x*y) { 
                
                // Adds edge all condition is passed
                board.addEdge(i, n);
            }
    }
}

void PrintResults(clock_t t, int x) {
    /*
    PURPOSE
    Print all the words found as well as the number of words and points

    PARAMETERS
    t (clock_t): The starting time
    x (int): The number of columns on the board

    COMPLEXITY
    O(n), Where n is the number of words found
    */

    // Iterate through all the words
    for (auto w : boggleWords) {
        // Output the word
        cout << w.first << " ";

        // Iterate through the path
        for (auto i : w.second) {
            // Print the path
            cout << "(" << i % x << ", " << i / x << ")";

            // If not last element print comma
            if (i != w.second.back())
                cout << ", ";
        }
        // Print a newline
        cout << endl;
    }

    // Print number of words, points, and run time
    cout << boggleWords.size() << " Words were found\n";
    cout << PossiblePoints() << " Points can be found\n";
    cout << (float)(clock() - t) / CLOCKS_PER_SEC << " Seconds to run\n";
}

bool CheckInput() {
    // Convert all letters to uppercase, then check if valid
    for (int i = 0; i < letters.length(); i++) {
        letters[i] = toupper(letters[i]);

        // False if a single letter fails
        if (letters[i] < 'A' || letters[i] > 'Z')
            return false;
    }

    // True if all letters pass
    return true;

}

void Setup(int x, int y, char ownBoard) {
    /*
    PURPOSE
    Create the trie and board

    PARAMETERS
    x (int): The number of columns on the board
    y (int): The number of rows on the board
    ownBoard (char): Indicates whether to randomply generate or get input
    */

    // Create an object of the Trie class, this object will hold dictionary information
    MakeTrie();

    // Create an object of the Digraph class, this object is a graph of a boggle board
    CreateBoard(x, y);

    getline(cin, letters);
    // Gets input for board letters if thats what the user input
    if (ownBoard == 'Y') {
        // Prompt and get letters
        do {
        cout << "Enter " << x*y << " Consecutive Letters: ";
        cin >> letters;
        } while (letters.length() != x*y || !CheckInput());      
    }
    else if (ownBoard == 'N') {
        // Randomly generates the board
        GenerateLetters(x * y);
    }

    // Prints the board out
    PrintBoard(x, y);
}

void Solver() {
    /*
    PURPOSE
    Generate or allow the user to input a board and solve it
    */

    // Declare a variable that determines if the board is random or input
    char ownBoard;

    // Get input for ownBoard
    do {
        // Print prompt message, get input
        cout << "Input your own board (Y/N): ";
        cin >> ownBoard;

        // Convert to uppercase
        ownBoard = toupper(ownBoard);

      // Loop until the input is valid
    } while (ownBoard != 'Y' && ownBoard != 'N');
    
    // Input dimensions of board
    int x, y;
    // Print prompt message, get input
    cout << "Enter Board Dimensions: ";
    cin >> x >> y;

    // Makes the trie and the board
    Setup(x, y, ownBoard);

    // Start the timer
    clock_t t = clock();

    // Solve the board
    SolveBoard(x * y);

    // Sort and eliminate words as long as the array is not empty
    if (boggleWords.size() > 0) {
        // Sort the words by length
        QuickSortLength(0, boggleWords.size() - 1);

        // Eliminate any word that appears more than once
        boggleWords = EliminateRepeats();
    }

    // Print the results
    PrintResults(t, x);
}

int main() {  
    // Enter the solver
    Solver();

    return 0;
}
