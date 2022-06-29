#pragma once
#include<vector>
#include"Graph.h"
using namespace std;
struct ROBDDNode
{
	int label; //-1 for leaf
	union
	{
		struct
		{
			ROBDDNode* true_branch;
			ROBDDNode* false_branch;
		}successor;
		int value;
	}value;
};
ROBDDNode* Clone(ROBDDNode* src);
bool Contain(ROBDDNode* node, int label);
class ROBDD
{
public:
	ROBDDNode* root;
	vector<ROBDDNode*> nodes;
	void ConvertFromGraph(Graph graph);
	void FromTrueValueVector(vector<int> TrueValues);
	void Simplify();
	void Print();
	ROBDD CloneROBDD();
	bool Walk(int path, int pathlen); //walk down the path, see if it ends.
};
vector<ROBDDNode*> NodeVector(ROBDDNode* StartVector);
bool Equal(ROBDDNode* node1, ROBDDNode* node2);
ROBDD AND(ROBDD robdd1, ROBDD robdd2);
ROBDD OR(ROBDD robdd1, ROBDD robdd2);
ROBDD IMPLY(ROBDD robdd1, ROBDD robdd2);
ROBDD NOT(ROBDD robdd);
ROBDD EX(Graph G, ROBDD robdd);
ROBDD EG(Graph G, ROBDD robdd);
ROBDD EU(Graph G, ROBDD robdd1, ROBDD robdd2);

