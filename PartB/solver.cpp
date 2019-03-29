#include <iostream> // Gives access to stdin, stdout

#include <string> // Gives access to strings
#include <vector> // Gives access to vecor data structures
#include <unordered_set> // Gives access to unordered_set

#include <ctime> // Gives access to random number generation
#include <time.h> // Gives access to timing the program
#include <fstream> // Gives access to files

#include "digraph.h" // Gives access to the Digraph class
#include "trie.h" // Gives access to the Trie class

vector<pair<string, vector<int>>> boggleWords; // Stores all words attainable on a given boggle board
unordered_set<string> wordsEntered;
Digraph board;
Trie dict; // Stores a given dictionary in a Trie data structure
string letters = ""; // Stores all the letters on a randomly generated boggle board


// Initialize all the files that we will need later
const string dictFile = "TextFiles/words.txt"; // from "http://www.gwicks.net/dictionaries.htm"
const string diceFile = "TextFiles/boggle-dice.txt"; // Contains all the dice information
 
#define NUM_SIDES_DIE 6 // Number of sides on a boggle die
#define NUM_DICE 16 // Number of dice on a board

using namespace std;

void SolveTile(string currWord, vector<int> tilesVisited, Digraph board) {
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
        SolveTile(currWord, tilesVisited, board);

        // Remove the letter appended
        currWord.pop_back();
        tilesVisited.pop_back();
    }

    // When we've iterated through all of a tiles neighbours, check to see if the tile is in dictionary
    if (dict.searchWord(currWord))
        boggleWords.push_back(pair<string, vector<int>>(currWord, tilesVisited));

    // End of function reached so it will recurse 1 layer out
}

void SolveBoard(Digraph board, int boardSize) {
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

void PrintBoard(int x, int y) {
    cout << "Board looks like:\n";
    //Prints out the word board
    for (int i = 0; i < x * y; i++) {
        cout << letters[i] << " ";
        if (i % x == x - 1) 
            cout << endl;
    }
    cout << endl;
}
// This wont be used for this file
void GenerateLetters(int numLetters) {

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

Digraph createBoard(int x, int y) {
    // Initialize an object of the Digraph class
    Digraph board;
    vector<int> neighbours;

    // Add a vertex at all blocks
    for (int i = 0; i < x * y; i++)
        board.addVertex(i);

    if (x == 1) {
        for (int i = 1; i < y - 1; i++) {
            board.addEdge(i, i + 1);
            board.addEdge(i + 1, i);
        }
        return board;
    }
    else if (y == 1) {
        for (int i = 1; i < x - 1; i++) {
            board.addEdge(i, i + 1);
            board.addEdge(i + 1, i);
        }
        return board;
    }

    for (int i = 0; i < x*y; i++) {
        if (x != 2) {
            // Initialize all the possible neighbours
            neighbours = {i - x - 1, i - x, i - x + 1, 
                          i - 1,                i + 1,
                          i + x - 1, i + x, i + x + 1};
        }

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
                board.addEdge(i, n);
            }
    }
    // Return the board that was created
    return board;
}

void PrintResults(clock_t t, int x) {
    for (auto w : boggleWords) {
        cout << w.first << " ";
        for (auto i : w.second) {
            cout << "(" << i % x << ", " << i / x << ")";

            if (i != w.second.back())
                cout << ", ";
        }


        cout << endl;
    }

    cout << boggleWords.size() << " Words were found\n";
    cout << (float)(clock() - t) / CLOCKS_PER_SEC << " Seconds to run\n";

}

void Setup(int x, int y) {
    // Create an object of the Trie class, this object will holds dictionary information
    dict = MakeTrie();

    // Create an object of the Digraph class, this object is a graph of a boggle board
    board = createBoard(x, y);

    // Randomly generates the board
    GenerateLetters(x * y);

    // Prints the board out
    PrintBoard(x, y);
}

void Solver() {
    // Input dimensions of board
    int x, y;
    cin >> x >> y;

    // Makes the trie and the board
    Setup(x, y);

    clock_t t = clock();

    //
    SolveBoard(board, x * y);

    if (boggleWords.size() > 0) {
        QuickSortLength(0, boggleWords.size() - 1);

        boggleWords = EliminateRepeats();
    }

    PrintResults(t, x);
}

int main() {  
    Solver();
    return 0;
}
