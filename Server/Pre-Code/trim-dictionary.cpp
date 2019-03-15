// This file was used to pre-process the dictionary
// It is not used in the actual server program in any capacity

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

int main() {
    vector<string> allWords;
    string word;
    ifstream dictionary("english3.txt"); // Reads in the file

    while (getline(dictionary, word)) {
        bool flag = false;
        for(int i = 0; i < word.length(); i++){
            if (word[i] < 97 || word[i] > 122)
                flag = true;
            word[i] -= 32;
        }
        if (flag)
            continue;
        if (word.length() < 3 || word.length() > 16)
            continue;

        allWords.push_back(word);
    }

    dictionary.close();
    
    ofstream shortDict("words2.txt");
    for (int i = 0; i < allWords.size(); i++) {
        shortDict << allWords[i] << endl;
    }
    shortDict.close();
}