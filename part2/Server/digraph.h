/*
Names: Brock Chelle, Benjamin Wagg
IDs: 1533398, 1531566
CCID: bchelle, bwagg
Course: CMPUT 275
Term: Winter 2019
Final Project: Boggle Solver (Part 2)
*/

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <set>

using namespace std;

/*
  Represents a graph using an adjacency list representation.
  Vertices are assumed to be integers.
*/
class Digraph {
public:
  // No constructor or destructor are necessary this time.
  // A new instance will be an empty graph with no nodes.

  // add a vertex, does nothing if it already exists
  void addVertex(int v);

  // adds an edge, creating the vertices if they do not exist
  // if the edge already existed, does nothing
  void addEdge(int u, int v);

  // Removes an edge from the board
  void removeEdge(int u, int v);

  // returns true if and only if v is a vertex in the graph
  bool isVertex(int v);

  // returns a const iterator to the neighbours of v
  unordered_set<int>::const_iterator neighbours(int v) const;

  // returns a const iterator to the end of v's neighour set
  unordered_set<int>::const_iterator endIterator(int v) const;


private:
  unordered_map<int, unordered_set<int>> nbrs;
};

void Digraph::addVertex(int v) {
  // If it already exists, does nothing.
  // Otherwise, adds v with an empty adjacency set.
  nbrs[v];
}

void Digraph::addEdge(int u, int v) {
  addVertex(v);
  nbrs[u].insert(v); // will also add u to nbrs if it was not there already
}

void Digraph::removeEdge(int u, int v) {
  // Removes the edge v from u
  nbrs[u].erase(nbrs[u].find(v));
}


bool Digraph::isVertex(int v) {
  return nbrs.find(v) != nbrs.end();
}


unordered_set<int>::const_iterator Digraph::neighbours(int v) const {
  // this will crash the program if v is not a vertex
  return nbrs.find(v)->second.begin();
}

unordered_set<int>::const_iterator Digraph::endIterator(int v) const {
  // this will crash the program if v is not a vertex
  return nbrs.find(v)->second.end();
}


#endif
