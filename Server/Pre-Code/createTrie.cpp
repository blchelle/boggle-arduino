// This file is not used for any part of the assignment
// It's only purpose was to test the implementation of the Trie data structure

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "trie.h"
#include <ctime>

const string filename = "words.txt";

using namespace std;

int main() {
    string word;
    ifstream wordFile(filename);
    Trie dictionary;

    vector<string> a;

    clock_t start;

    start = clock();

    while (getline(wordFile, word)) {
        dictionary.insert(word);
        a.push_back(word);
    }

    int count = 0; 
    for (auto i : a) {
        if (dictionary.searchWord(i))
        count++;
    }

    cout << "Number of words found: " << count << endl;
    cout << "Time: " << (clock() - start) / (double) 1000000 << endl;
}