#ifndef _TRIE_H_
#define _TRIE_H_

#include <string>
#define NUM_CHARS 128

using namespace std;

class Trie {
public: 
    // Will store whether or not the Trie object is a word that belongs to a dictionary
    bool isWord;

    // An array of pointers to an objects of the Trie class
    Trie* character[NUM_CHARS]; 

    // Constructor for the tree
    Trie() {
        //Initialize properties of the trie
        this->isWord = false;

        // Initialize each pointer to point to NULL 
        for (int i = 0; i < NUM_CHARS; i++) {
            this->character[i] = NULL;
        }
    }

    void insert(string);
    bool searchWord(string);
};

void Trie::insert(string word) {
    // Start from the root node
    Trie* currNode = this;

    for (int i = 0; i < word.length(); i++) {
        // If branch doesn't exist, create new node
        if (currNode->character[word[i]] == NULL)
            currNode->character[word[i]] = new Trie();

        // Travel to next node
        currNode = currNode->character[word[i]];
    }

    // When done, mark the current node as a leaf.
    // Every word in the dictionary will end up being a leaf
    currNode->isWord = true;
}

/* Due to the way that our code progresses, we've already guaranteed that the prefix is
   in the tree, so all wee need to do is travel to the node and see if its a leaf */
bool Trie::searchWord(string word, bool prefix) {
    Trie* currNode = this;

    for (int i = 0; i < word.length(); i++) {
        // Travel to the next node
        currNode = currNode->character[word[i]];

        // If our prefix falls off of the trie then it is not valid
        if (currNode == NULL) 
            return false;
    }

    if (!prefix)
        return currNode->isWord;
    else
        return true;
}


#endif