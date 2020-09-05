/*
Names: Brock Chelle, Benjamin Wagg
IDs: 1533398, 1531566
CCID: bchelle, bwagg
Course: CMPUT 275
Term: Winter 2019
Final Project: Boggle Solver (Part 2)
*/

#ifndef _TRIE_H_
#define _TRIE_H_

#include <string>
#define NUM_CHARS 26
#define A_ASCII 65

using namespace std;

class Trie {
public:
    // Will store whether or not the Trie object is a word that belongs to a dictionary
    bool isWord;

    // An array of pointers to an objects of the Trie class
    Trie* character[NUM_CHARS];

    // Constructor for the tree
    Trie() {
        //Initialize properties of the trie, root node to false
        this->isWord = false;

        // Initialize each pointer to point to NULL
        for (int i = 0; i < NUM_CHARS; i++) {
            this->character[i] = NULL;
        }
    }

    void insert(string);
    bool searchWord(string, bool);
};

void Trie::insert(string word) {
    /*
    PURPOSE
    Insert a word into the trie object

    PARAMETERS
    word (string): The word being inserted into the string
    */

    // Start from the root node
    Trie* currNode = this;

    for (int i = 0; i < word.length(); i++) {
        // If branch doesn't exist, create new node
        if (currNode->character[word[i] - A_ASCII] == NULL)
            currNode->character[word[i] - A_ASCII] = new Trie();

        // Travel to next node
        currNode = currNode->character[word[i] - A_ASCII];
    }

    // When done, mark the current node as a leaf.
    // Every complete word in the dictionary will end up being a leaf
    currNode->isWord = true;
}

bool Trie::searchWord(string word, bool prefix) {
    /*
    PURPOSE
    Checks to see if a given word or prefix exits in the trie

    PARAMETERS
    word (string): The word being searched for
    prefix (bool): whether we're checking for a prefix or a full word

    RETURNS
    (bool): true or false depending on if the word/prefix exists in the tree
    */

    // Point to the root node
    Trie* currNode = this;

    // Iterate through the letters of the word
    for (int i = 0; i < word.length(); i++) {

        // Travel to the next node
        currNode = currNode->character[word[i] - A_ASCII];

        // If our prefix falls off of the trie then it is not valid
        if (currNode == NULL)
            return false;
    }
    // If seaching for a full word then return whether the node is a full word
    if (!prefix)
        return currNode->isWord;
    // If searching for a prefix, return true
    else
        return true;
}

#endif