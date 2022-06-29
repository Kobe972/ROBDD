#pragma once
#include<vector>
#include<cstdlib>
using namespace std;
struct GraphNode
{
	int value;
	vector<GraphNode*> next;
	vector<int> nextidx; //corresponding index
};
class Graph
{
public:
	int num_nodes;
	vector<GraphNode*> nodes;
	Graph();
	void AddNode(int value);
	void AddEdge(int nodesrc, int nodedst); //Add an edge from nodesrc to nodedst
};