#include "Graph.h"

Graph::Graph()
{
	num_nodes = 0;
}

void Graph::AddNode(int value)
{
	num_nodes++;
	GraphNode* NewNode = new GraphNode;
	nodes.push_back(NewNode);
	nodes[num_nodes - 1]->value = value;
}

void Graph::AddEdge(int nodesrc, int nodedst)
{
	if (nodesrc < 0 || nodedst < 0 || nodesrc >= num_nodes || nodedst >= num_nodes) throw "Added an illegal edge!";
	nodes[nodesrc]->next.push_back(nodes[nodedst]);
	nodes[nodesrc]->nextidx.push_back(nodedst);
}
