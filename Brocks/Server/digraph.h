//DO NOT MODIFY THIS FILE
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

  // returns true if and only if (u,v) is an edge in the graph
  // will certainly return false if neither vertex is in the graph
  bool isEdge(int u, int v);

  // returns a const iterator to the neighbours of v
  unordered_set<int>::const_iterator neighbours(int v) const;

  // returns a const iterator to the end of v's neighour set
  unordered_set<int>::const_iterator endIterator(int v) const;

  // return the number of outgoing neighbours of v
  int numNeighbours(int v);

  // returns the number of nodes
  int size();

  // return a vector with all vertices
  vector<int> vertices();

  //unordered_map<int, char> pairs();
  
  // returns true if 'walk' represents a walk on this graph
  // A walk is a sequence of vertices (perhaps with repeated vertices) 
  // v0, v1, . . . , vk where (vi,vi+1) is an edge for each 0 <= i < k.
  // the length of a walk is the number of edges traversed 
  bool isWalk(vector<int> walk);

  // returns true if 'path' represents a path on this graph
  // a path is a walk with no repeated vertices
  bool isPath(vector<int> path);

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

bool Digraph::isEdge(int u, int v) {
  // check that u is in the keys and that it's associated set contains v
  return nbrs.find(u) != nbrs.end() && nbrs[u].find(v) != nbrs[u].end();
}

unordered_set<int>::const_iterator Digraph::neighbours(int v) const {
  // this will crash the program if v is not a vertex
  return nbrs.find(v)->second.begin();
}

unordered_set<int>::const_iterator Digraph::endIterator(int v) const {
  // this will crash the program if v is not a vertex
  return nbrs.find(v)->second.end();
}

int Digraph::numNeighbours(int v) {
  // this will crash the program if v is not a vertex
  return nbrs.find(v)->second.size();
}

int Digraph::size() {
  return nbrs.size();
}

vector<int> Digraph::vertices() {
  vector<int> v;
  for (unordered_map<int, unordered_set<int>>::const_iterator it = nbrs.begin();
    it != nbrs.end(); it++) {
      v.push_back(it->first);
    }
  return v;
}
/*
unordered_map<int, char> Digraph::pairs() {
  unordered_map<int, char> m;
  for(unordered_map<int, char>::const_iterator it = p.begin(); it != p.end(); it++) {

  }
  return m;
}
*/

bool Digraph::isWalk(vector<int> walk) {
  if (walk.size() == 0)
    return false;
  if (walk.size() == 1)
    return isVertex(walk[0]);
  for (vector<int>::size_type i=0; i<walk.size()-1; i++)
    if (!isEdge(walk[i], walk[i+1]))
      return false;

  return true;
}

bool Digraph::isPath(vector<int> path) {
  set<int> s(path.begin(), path.end());
  if (s.size() < path.size())
    return false;

  return isWalk(path);
}

#endif
