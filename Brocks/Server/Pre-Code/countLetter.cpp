// This file is not used in the actual program
// This file counts the frequency of each letter and assigns odds 
// for weighted random dice rolls

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <utility>

using namespace std;

int main() {
    vector<string> allWords;
    string word;
    ifstream dictionary("words.txt"); // Reads in the file
    map<char, int> letterCount;

    string allLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for(int i = 0; i < allLetters.length(); i++) {
        letterCount.insert(pair<char, int> (allLetters[i], 0));
    }

    while (getline(dictionary, word)) {
        for (int i = 0; i < word.length(); i++)
            letterCount[word[i]]++;
    }

    dictionary.close();
    
    ofstream shortDict("letter-count.txt");
    for (auto i: letterCount) {
        shortDict << i.first << "," <<  i.second << endl;
    }
    shortDict.close();
}