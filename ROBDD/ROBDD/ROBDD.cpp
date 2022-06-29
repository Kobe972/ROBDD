#include "ROBDD.h"
#include "MathFunc.h"
#include <math.h>
#include <map>
#include <iostream>
using namespace std;

void ROBDD::ConvertFromGraph(Graph graph)
{
	if (!nodes.empty()) nodes.clear();
	root = new ROBDDNode; //root
	root->value.successor.true_branch = root->value.successor.false_branch = NULL;
	root->label = 0;
	nodes.push_back(root);
	int depth = ceil(log2(graph.num_nodes));
	ROBDDNode* current;
	for (int i = 0; i < graph.num_nodes; i++)
	{
		if (graph.nodes[i]->value == 0) continue;
		vector<bool> path = IntToBinVec(i, depth);
		current = root;
		for (int j = 0; j < path.size(); j++)
		{
			if (path[j] == true)  //walk to true branch
			{
				if (current->value.successor.true_branch == NULL) //create a new node
				{
					ROBDDNode* NewNode = new ROBDDNode;
					current->label = j;
					current->value.successor.true_branch = NewNode;
					NewNode->label = j + 1;
					NewNode->value.successor.true_branch = NULL;
					NewNode->value.successor.false_branch = NULL;
					nodes.push_back(NewNode);
					current = NewNode;
				}
				else //move down current
				{
					current = current->value.successor.true_branch;
				}
			}
			else //walk to false branch
			{
				if (current->value.successor.false_branch == NULL) //create a new node
				{
					ROBDDNode* NewNode = new ROBDDNode;
					current->label = j;
					current->value.successor.false_branch = NewNode;
					NewNode->value.successor.true_branch = NULL;
					NewNode->value.successor.false_branch = NULL;
					NewNode->label = j + 1;
					nodes.push_back(NewNode);
					current = NewNode;
				}
				else //move down current
				{
					current = current->value.successor.false_branch;
				}
			}
		}
		current->label = -1; //convert the last node to leaf
		current->value.value = 1;
	}
	vector<ROBDDNode*> NodeToAdd;
	for (int i = 0; i < nodes.size(); i++) //assign false leaves
	{
		if (nodes[i]->label != -1 && nodes[i]->value.successor.true_branch == NULL)
		{
			ROBDDNode* NewNode = new ROBDDNode;
			NewNode->label = -1;
			NewNode->value.value = 0;
			nodes[i]->value.successor.true_branch = NewNode;
			NodeToAdd.push_back(NewNode);
		}
		if (nodes[i]->label != -1 && nodes[i]->value.successor.false_branch == NULL)
		{
			ROBDDNode* NewNode = new ROBDDNode;
			NewNode->label = -1;
			NewNode->value.value = 0;
			nodes[i]->value.successor.false_branch = NewNode;
			NodeToAdd.push_back(NewNode);
		}
	}
	for (int i = 0; i < NodeToAdd.size(); i++) nodes.push_back(NodeToAdd[i]);
	Simplify();
}

void ROBDD::FromTrueValueVector(vector<int> TrueValues)
{
	if (!nodes.empty()) nodes.clear();
	root = new ROBDDNode; //root
	root->value.successor.true_branch = root->value.successor.false_branch = NULL;
	root->label = 0;
	nodes.push_back(root);
	int max = 0;
	for (int i = 0; i < TrueValues.size(); i++)
	{
		if (TrueValues[i] > max) max = TrueValues[i];
	}
	int depth = ceil(log2(max + 1));
	ROBDDNode* current;
	for (int i = 0; i < TrueValues.size(); i++)
	{
		vector<bool> path = IntToBinVec(TrueValues[i], depth);
		current = root;
		for (int j = 0; j < path.size(); j++)
		{
			if (path[j] == true)  //walk to true branch
			{
				if (current->value.successor.true_branch == NULL) //create a new node
				{
					ROBDDNode* NewNode = new ROBDDNode;
					current->label = j;
					current->value.successor.true_branch = NewNode;
					NewNode->label = j + 1;
					NewNode->value.successor.true_branch = NULL;
					NewNode->value.successor.false_branch = NULL;
					nodes.push_back(NewNode);
					current = NewNode;
				}
				else //move down current
				{
					current = current->value.successor.true_branch;
				}
			}
			else //walk to false branch
			{
				if (current->value.successor.false_branch == NULL) //create a new node
				{
					ROBDDNode* NewNode = new ROBDDNode;
					current->label = j;
					current->value.successor.false_branch = NewNode;
					NewNode->value.successor.true_branch = NULL;
					NewNode->value.successor.false_branch = NULL;
					NewNode->label = j + 1;
					nodes.push_back(NewNode);
					current = NewNode;
				}
				else //move down current
				{
					current = current->value.successor.false_branch;
				}
			}
		}
		current->label = -1; //convert the last node to leaf
		current->value.value = 1;
	}
	vector<ROBDDNode*> NodeToAdd;
	for (int i = 0; i < nodes.size(); i++) //assign false leaves
	{
		if (nodes[i]->label != -1 && nodes[i]->value.successor.true_branch == NULL)
		{
			ROBDDNode* NewNode = new ROBDDNode;
			NewNode->label = -1;
			NewNode->value.value = 0;
			nodes[i]->value.successor.true_branch = NewNode;
			NodeToAdd.push_back(NewNode);
		}
		if (nodes[i]->label != -1 && nodes[i]->value.successor.false_branch == NULL)
		{
			ROBDDNode* NewNode = new ROBDDNode;
			NewNode->label = -1;
			NewNode->value.value = 0;
			nodes[i]->value.successor.false_branch = NewNode;
			NodeToAdd.push_back(NewNode);
		}
	}
	for (int i = 0; i < NodeToAdd.size(); i++) nodes.push_back(NodeToAdd[i]);
	Simplify();
}

void ROBDD::Simplify() //label nannot be less than -1
{
	int flag;
	do
	{
		flag = 0;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i]->label != -1 && Equal(nodes[i]->value.successor.true_branch, nodes[i]->value.successor.false_branch))
			{
				nodes[i]->label = nodes[i]->value.successor.true_branch->label;
				if (nodes[i]->value.successor.true_branch->label == -1)
					nodes[i]->value.value = nodes[i]->value.successor.true_branch->value.value;
				else
				{
					nodes[i]->value.successor.true_branch = nodes[i]->value.successor.false_branch->value.successor.true_branch;
					nodes[i]->value.successor.false_branch = nodes[i]->value.successor.false_branch->value.successor.false_branch;
				}
				flag = 1;
			}
		}
		nodes = NodeVector(root);
		for (int i = 0; i < nodes.size(); i++) //merge
		{
			for (int j = i + 1; j < nodes.size(); j++)
			{
				if (nodes[i]->label == -1 || nodes[j]->label == -1) continue;
				if (nodes[j]->value.successor.false_branch != nodes[i]->value.successor.false_branch&&Equal(nodes[j]->value.successor.false_branch, nodes[i]->value.successor.false_branch))
				{
					flag = 1;
					for (vector<ROBDDNode*>::iterator ite = nodes.begin(); ite != nodes.end(); ite++)
					{
						if (*ite == nodes[j]->value.successor.false_branch)
						{
							nodes[j]->value.successor.false_branch = nodes[i]->value.successor.false_branch;
							goto erase;
						}
					}
				}
				if (nodes[j]->value.successor.false_branch != nodes[i]->value.successor.true_branch&&Equal(nodes[j]->value.successor.false_branch, nodes[i]->value.successor.true_branch))
				{
					flag = 1;
					for (vector<ROBDDNode*>::iterator ite = nodes.begin(); ite != nodes.end(); ite++)
					{
						if (*ite == nodes[j]->value.successor.false_branch)
						{
							nodes[j]->value.successor.false_branch = nodes[i]->value.successor.true_branch;
							goto erase;
						}
					}
				}
				if (nodes[j]->value.successor.true_branch != nodes[i]->value.successor.false_branch&&Equal(nodes[j]->value.successor.true_branch, nodes[i]->value.successor.false_branch))
				{
					flag = 1;
					for (vector<ROBDDNode*>::iterator ite = nodes.begin(); ite != nodes.end(); ite++)
					{
						if (*ite == nodes[j]->value.successor.true_branch)
						{
							nodes[j]->value.successor.true_branch = nodes[i]->value.successor.false_branch;
							goto erase;
						}
					}
				}
				if (nodes[j]->value.successor.true_branch != nodes[i]->value.successor.true_branch&&Equal(nodes[j]->value.successor.true_branch, nodes[i]->value.successor.true_branch))
				{
					flag = 1;
					for (vector<ROBDDNode*>::iterator ite = nodes.begin(); ite != nodes.end(); ite++)
					{
						if (*ite == nodes[j]->value.successor.true_branch)
						{
							nodes[j]->value.successor.true_branch = nodes[i]->value.successor.true_branch;
							goto erase;
						}
					}
				}
			}
		}
	erase:;
		nodes = NodeVector(root);
	} while (flag);
}

void ROBDD::Print() //label cannot be less than -1
{
	map<ROBDDNode*, int> ID;
	for (int i = 0; i < nodes.size(); i++) ID[nodes[i]] = i;
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->label != -1)
		{
			if (nodes[i]->value.successor.false_branch->label != -1)
				cout << ID[nodes[i]] << "(tests x" << nodes[i]->label << ")" << "  ----False---->  " << ID[nodes[i]->value.successor.false_branch] << endl;
			else
				if (nodes[i]->value.successor.false_branch->value.value == 1)
					cout << ID[nodes[i]] << "(tests x" << nodes[i]->label << ")" << "  ----False---->  " << ID[nodes[i]->value.successor.false_branch] << "(True)" << endl;
				else
					cout << ID[nodes[i]] << "(tests x" << nodes[i]->label << ")" << "  ----False---->  " << ID[nodes[i]->value.successor.false_branch] << "(False)" << endl;
			if (nodes[i]->value.successor.true_branch->label != -1)
				cout << ID[nodes[i]] << "(tests x" << nodes[i]->label << ")" << "  ----True---->  " << ID[nodes[i]->value.successor.true_branch] << endl;
			else
				if (nodes[i]->value.successor.true_branch->value.value == 1)
					cout << ID[nodes[i]] << "(tests x" << nodes[i]->label << ")" << "  ----True---->  " << ID[nodes[i]->value.successor.true_branch] << "(True)" << endl;
				else
					cout << ID[nodes[i]] << "(tests x" << nodes[i]->label << ")" << "  ----True---->  " << ID[nodes[i]->value.successor.true_branch] << "(False)" << endl;
		}
		else
		{
			if (nodes[i]->value.value == 1)
				cout << ID[nodes[i]] << "  stands for True" << endl;
			else
				cout << ID[nodes[i]] << "  stands for False" << endl;
		}
	}
}

ROBDD ROBDD::CloneROBDD()
{
	ROBDD ret;
	ret.root = Clone(root);
	ret.nodes = NodeVector(ret.root);
	ret.Simplify();
	return ret;
}

bool ROBDD::Walk(int path, int pathlen)
{
	vector<bool> _path = IntToBinVec(path, pathlen);
	ROBDDNode* current = root;
	for (int i = 0; i < _path.size(); i++)
	{
		if (i != current->label) continue;
		if (_path[i]) current = current->value.successor.true_branch;
		else current = current->value.successor.false_branch;
	}
	if (current->value.value == 0) return false;
	return true;
}

vector<ROBDDNode*> NodeVector(ROBDDNode * StartVector)
{
	vector<ROBDDNode*> ret, TrueBranch, FalseBranch;
	ret.push_back(StartVector);
	if (StartVector->label >= 0)
	{
		TrueBranch = NodeVector(StartVector->value.successor.true_branch);
		FalseBranch = NodeVector(StartVector->value.successor.false_branch);
		for (int i = 0; i < TrueBranch.size(); i++)
		{
			vector<ROBDDNode*>::iterator it;
			it = find(ret.begin(), ret.end(), TrueBranch[i]);
			if (it != ret.end()) continue;
			ret.push_back(TrueBranch[i]);
		}
		for (int i = 0; i < FalseBranch.size(); i++)
		{
			vector<ROBDDNode*>::iterator it;
			it = find(ret.begin(), ret.end(), FalseBranch[i]);
			if (it != ret.end()) continue;
			ret.push_back(FalseBranch[i]);
		}
	}
	return ret;
}

bool Equal(ROBDDNode* node1, ROBDDNode* node2) //label cannot be less than -1
{
	if (node1->label == -1 && node2->label == -1)
	{
		if (node1->value.value == node2->value.value) return true;
		else return false;
	}
	else if (node1->label == -1 || node2->label == -1) return false;
	else return Equal(node1->value.successor.true_branch, node2->value.successor.true_branch) & Equal(node1->value.successor.false_branch, node2->value.successor.false_branch);
}

ROBDD AND(ROBDD robdd1, ROBDD robdd2)
{
	ROBDD cloned_left = robdd1.CloneROBDD();
	ROBDD cloned_right = robdd2.CloneROBDD();
	ROBDD robdd_false;
	ROBDDNode* NewNode = new ROBDDNode;
	NewNode->label = -1;
	NewNode->value.value = 0;
	robdd_false.nodes.push_back(NewNode);
	robdd_false.root = NewNode;
	if (cloned_left.nodes.size() == 1)
	{
		if (cloned_left.root->value.value == 1) return cloned_right;
		else return robdd_false;
	}
	if (cloned_right.nodes.size() == 1)
	{
		if (cloned_right.root->value.value == 1) return cloned_left;
		else return robdd_false;
	}
	if (robdd1.root->label == robdd2.root->label)
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_left_left, cloned_left_right, cloned_right_left, cloned_right_right;
		NewNode = Clone(cloned_left.root->value.successor.true_branch);
		cloned_left_left.root = NewNode;
		cloned_left_left.nodes = NodeVector(cloned_left_left.root);
		NewNode = Clone(cloned_left.root->value.successor.false_branch);
		cloned_left_right.root = NewNode;
		cloned_left_right.nodes = NodeVector(cloned_left_right.root);
		NewNode = Clone(cloned_right.root->value.successor.true_branch);
		cloned_right_left.root = NewNode;
		cloned_right_left.nodes = NodeVector(cloned_right_left.root);
		NewNode = Clone(cloned_right.root->value.successor.false_branch);
		cloned_right_right.root = NewNode;
		cloned_right_right.nodes = NodeVector(cloned_right_right.root);
		TrueROBDD = AND(cloned_left_left, cloned_right_left);
		FalseROBDD = AND(cloned_left_right, cloned_right_right);
		ROBDD ret;
		ret.root = new ROBDDNode;
		ret.root->label = robdd1.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
		ret.nodes = NodeVector(ret.root);
		ret.Simplify();
		return ret;
	}
	ROBDD ret;
	if (!Contain(cloned_right.root, cloned_left.root->label))
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_left_left, cloned_left_right;
		NewNode = Clone(cloned_left.root->value.successor.true_branch);
		cloned_left_left.root = NewNode;
		cloned_left_left.nodes = NodeVector(cloned_left_left.root);
		NewNode = Clone(cloned_left.root->value.successor.false_branch);
		cloned_left_right.root = NewNode;
		cloned_left_right.nodes = NodeVector(cloned_left_right.root);
		TrueROBDD = AND(cloned_left_left, cloned_right);
		FalseROBDD = AND(cloned_left_right, cloned_right);
		ret.root = new ROBDDNode;
		ret.root->label = robdd1.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
	}
	else
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_right_left, cloned_right_right;
		NewNode = Clone(cloned_right.root->value.successor.true_branch);
		cloned_right_left.root = NewNode;
		cloned_right_left.nodes = NodeVector(cloned_right_left.root);
		NewNode = Clone(cloned_right.root->value.successor.false_branch);
		cloned_right_right.root = NewNode;
		cloned_right_right.nodes = NodeVector(cloned_right_right.root);
		TrueROBDD = AND(cloned_right_left, cloned_left);
		FalseROBDD = AND(cloned_right_right, cloned_left);
		ret.root = new ROBDDNode;
		ret.root->label = robdd2.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
	}
	ret.nodes = NodeVector(ret.root);
	for (int i = 0; i < ret.nodes.size(); i++)
	{
		if (ret.nodes[i]->label == 0)
		{
			ret.root = ret.nodes[i];
			break;
		}
	}
	ret.nodes = NodeVector(ret.root);
	ret.Simplify();
	return ret;
}

ROBDD OR(ROBDD robdd1, ROBDD robdd2)
{
	ROBDD cloned_left = robdd1.CloneROBDD();
	ROBDD cloned_right = robdd2.CloneROBDD();
	ROBDD robdd_true;
	ROBDDNode* NewNode = new ROBDDNode;
	NewNode->label = -1;
	NewNode->value.value = 1;
	robdd_true.nodes.push_back(NewNode);
	robdd_true.root = NewNode;
	if (cloned_left.nodes.size() == 1)
	{
		if (cloned_left.root->value.value == 1) return robdd_true;
		else return cloned_right;
	}
	if (cloned_right.nodes.size() == 1)
	{
		if (cloned_right.root->value.value == 1) return robdd_true;
		else return cloned_left;
	}
	if (robdd1.root->label == robdd2.root->label)
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_left_left, cloned_left_right, cloned_right_left, cloned_right_right;
		NewNode = Clone(cloned_left.root->value.successor.true_branch);
		cloned_left_left.root = NewNode;
		cloned_left_left.nodes = NodeVector(cloned_left_left.root);
		NewNode = Clone(cloned_left.root->value.successor.false_branch);
		cloned_left_right.root = NewNode;
		cloned_left_right.nodes = NodeVector(cloned_left_right.root);
		NewNode = Clone(cloned_right.root->value.successor.true_branch);
		cloned_right_left.root = NewNode;
		cloned_right_left.nodes = NodeVector(cloned_right_left.root);
		NewNode = Clone(cloned_right.root->value.successor.false_branch);
		cloned_right_right.root = NewNode;
		cloned_right_right.nodes = NodeVector(cloned_right_right.root);
		TrueROBDD = OR(cloned_left_left, cloned_right_left);
		FalseROBDD = OR(cloned_left_right, cloned_right_right);
		ROBDD ret;
		ret.root = new ROBDDNode;
		ret.root->label = robdd1.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
		ret.nodes = NodeVector(ret.root);
		ret.Simplify();
		return ret;
	}
	ROBDD ret;
	if (!Contain(cloned_right.root, cloned_left.root->label))
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_left_left, cloned_left_right;
		NewNode = Clone(cloned_left.root->value.successor.true_branch);
		cloned_left_left.root = NewNode;
		cloned_left_left.nodes = NodeVector(cloned_left_left.root);
		NewNode = Clone(cloned_left.root->value.successor.false_branch);
		cloned_left_right.root = NewNode;
		cloned_left_right.nodes = NodeVector(cloned_left_right.root);
		TrueROBDD = OR(cloned_left_left, cloned_right);
		FalseROBDD = OR(cloned_left_right, cloned_right);
		ret.root = new ROBDDNode;
		ret.root->label = robdd1.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
	}
	else
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_right_left, cloned_right_right;
		NewNode = Clone(cloned_right.root->value.successor.true_branch);
		cloned_right_left.root = NewNode;
		cloned_right_left.nodes = NodeVector(cloned_right_left.root);
		NewNode = Clone(cloned_right.root->value.successor.false_branch);
		cloned_right_right.root = NewNode;
		cloned_right_right.nodes = NodeVector(cloned_right_right.root);
		TrueROBDD = OR(cloned_right_left, cloned_left);
		FalseROBDD = OR(cloned_right_right, cloned_left);
		ret.root = new ROBDDNode;
		ret.root->label = robdd2.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
	}
	ret.nodes = NodeVector(ret.root);
	for (int i = 0; i < ret.nodes.size(); i++)
	{
		if (ret.nodes[i]->label == 0)
		{
			ret.root = ret.nodes[i];
			break;
		}
	}
	ret.nodes = NodeVector(ret.root);
	ret.Simplify();
	return ret;
}

ROBDD IMPLY(ROBDD robdd1, ROBDD robdd2)
{
	ROBDD cloned_left = robdd1.CloneROBDD();
	ROBDD cloned_right = robdd2.CloneROBDD();
	ROBDD robdd_true;
	ROBDDNode* NewNode = new ROBDDNode;
	NewNode->label = -1;
	NewNode->value.value = 1;
	robdd_true.nodes.push_back(NewNode);
	robdd_true.root = NewNode;
	if (cloned_left.nodes.size() == 1)
	{
		if (cloned_left.root->value.value == 1) return cloned_right;
		else return robdd_true;
	}
	if (cloned_right.nodes.size() == 1)
	{
		if (cloned_right.root->value.value == 1) return robdd_true;
		else return cloned_left;
	}
	if (robdd1.root->label == robdd2.root->label)
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_left_left, cloned_left_right, cloned_right_left, cloned_right_right;
		NewNode = Clone(cloned_left.root->value.successor.true_branch);
		cloned_left_left.root = NewNode;
		cloned_left_left.nodes = NodeVector(cloned_left_left.root);
		NewNode = Clone(cloned_left.root->value.successor.false_branch);
		cloned_left_right.root = NewNode;
		cloned_left_right.nodes = NodeVector(cloned_left_right.root);
		NewNode = Clone(cloned_right.root->value.successor.true_branch);
		cloned_right_left.root = NewNode;
		cloned_right_left.nodes = NodeVector(cloned_right_left.root);
		NewNode = Clone(cloned_right.root->value.successor.false_branch);
		cloned_right_right.root = NewNode;
		cloned_right_right.nodes = NodeVector(cloned_right_right.root);
		TrueROBDD = IMPLY(cloned_left_left, cloned_right_left);
		FalseROBDD = IMPLY(cloned_left_right, cloned_right_right);
		ROBDD ret;
		ret.root = new ROBDDNode;
		ret.root->label = robdd1.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
		ret.nodes = NodeVector(ret.root);
		for (int i = 0; i < ret.nodes.size(); i++)
		{
			if (ret.nodes[i]->label == 0)
			{
				ret.root = ret.nodes[i];
				break;
			}
		}
		ret.nodes = NodeVector(ret.root);
		ret.Simplify();
		return ret;
	}
	ROBDD ret;
	if (!Contain(cloned_right.root, cloned_left.root->label))
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_left_left, cloned_left_right;
		NewNode = Clone(cloned_left.root->value.successor.true_branch);
		cloned_left_left.root = NewNode;
		cloned_left_left.nodes = NodeVector(cloned_left_left.root);
		NewNode = Clone(cloned_left.root->value.successor.false_branch);
		cloned_left_right.root = NewNode;
		cloned_left_right.nodes = NodeVector(cloned_left_right.root);
		TrueROBDD = IMPLY(cloned_left_left, cloned_right);
		FalseROBDD = IMPLY(cloned_left_right, cloned_right);
		ret.root = new ROBDDNode;
		ret.root->label = robdd1.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
	}
	else
	{
		ROBDD TrueROBDD, FalseROBDD;
		ROBDD cloned_right_left, cloned_right_right;
		NewNode = Clone(cloned_right.root->value.successor.true_branch);
		cloned_right_left.root = NewNode;
		cloned_right_left.nodes = NodeVector(cloned_right_left.root);
		NewNode = Clone(cloned_right.root->value.successor.false_branch);
		cloned_right_right.root = NewNode;
		cloned_right_right.nodes = NodeVector(cloned_right_right.root);
		TrueROBDD = IMPLY(cloned_right_left, cloned_left);
		FalseROBDD = IMPLY(cloned_right_right, cloned_left);
		ret.root = new ROBDDNode;
		ret.root->label = robdd2.root->label;
		ret.root->value.successor.true_branch = TrueROBDD.root;
		ret.root->value.successor.false_branch = FalseROBDD.root;
	}
	ret.nodes = NodeVector(ret.root);
	ret.Simplify();
	return ret;
}

ROBDD NOT(ROBDD robdd)
{
	ROBDD ret = robdd.CloneROBDD();
	ret.nodes = NodeVector(ret.root);
	for (int i = 0; i < ret.nodes.size(); i++)
	{
		if (ret.nodes[i]->label == -1) ret.nodes[i]->value.value = 1 - ret.nodes[i]->value.value;
	}
	ret.Simplify();
	return ret;
}

ROBDD EG(Graph G, ROBDD robdd)
{ //V = {s ∈ T | ∃t ∈ U : s → t}
	cout << "\nImplementing EG..." << endl;
	int finished = 0;
	ROBDD T = robdd.CloneROBDD();
	ROBDD t0 = robdd.CloneROBDD();
	ROBDD tn = t0.CloneROBDD();
	int epoch = 0;
	while (!finished)
	{
		cout << "\nEpoch " << epoch << endl;
		ROBDD U = tn.CloneROBDD();
		cout << "\nt" << epoch << ":" << endl;
		tn.Print();
		vector<int> P1_table;
		int depth = ceil(log2(G.num_nodes));
		for (int i = 0; i < G.num_nodes; i++)
		{
			for (int j = 0; j < G.nodes[i]->next.size(); j++)
			{
				P1_table.push_back((i << depth) + G.nodes[i]->nextidx[j]);
			}
		}
		ROBDD P1, P2 = U.CloneROBDD();
		P1.FromTrueValueVector(P1_table);
		for (int i = 0; i < P2.nodes.size(); i++)
		{
			if (P2.nodes[i]->label >= 0) P2.nodes[i]->label += depth;
		}
		cout << "\nP1:" << endl;
		P1.Print();
		cout << "\nP2:" << endl;
		P2.Print();
		ROBDD P = AND(P1, P2);
		cout << "\nP:" << endl;
		P.Print();
		vector<int> SPe_table;
		for (int i = 0; i < pow(2,depth*2); i++)
		{
			if (P.Walk(i, depth * 2)) SPe_table.push_back(i >> depth);
		}
		ROBDD SPe;
		SPe.FromTrueValueVector(SPe_table);
		cout << "\nSPe:" << endl;
		SPe.Print();
		cout << "\nT:" << endl;
		T.Print();
		ROBDD V = AND(T, SPe);
		cout << "\nV:" << endl;
		V.Print();
		ROBDD last = tn.CloneROBDD();
		tn = AND(tn, V);
		if (Equal(tn.root, last.root))
		{
			cout << "\ntn=tn-1" << endl;
			finished = 1;
		}
		epoch++;
	}
	return tn;
}

ROBDD EX(Graph G, ROBDD robdd)
{ //V = {s ∈ T | ∃t ∈ U : s → t}
	cout << "\nImplementing EX..." << endl;
	int finished = 0;
	int depth = ceil(log2(G.num_nodes));
	ROBDD U = robdd.CloneROBDD();
	ROBDD T;
	vector<int> T_tables;
	for (int i = 0; i < G.num_nodes; i++)
	{
		T_tables.push_back(i);
	}
	T.FromTrueValueVector(T_tables);
	ROBDD tn = U.CloneROBDD();
	cout << "\nT:" << endl;
	T.Print();
	vector<int> P1_table;
	for (int i = 0; i < G.num_nodes; i++)
	{
		for (int j = 0; j < G.nodes[i]->next.size(); j++)
		{
			P1_table.push_back((i << depth) + G.nodes[i]->nextidx[j]);
		}
	}
	ROBDD P1, P2 = U.CloneROBDD();
	P1.FromTrueValueVector(P1_table);
	for (int i = 0; i < P2.nodes.size(); i++)
	{
		if (P2.nodes[i]->label >= 0) P2.nodes[i]->label += depth;
	}
	cout << "\nP1:" << endl;
	P1.Print();
	cout << "\nP2:" << endl;
	P2.Print();
	ROBDD P = AND(P1, P2);
	cout << "\nP:" << endl;
	P.Print();
	vector<int> SPe_table;
	for (int i = 0; i < pow(2, depth * 2); i++)
	{
		if (P.Walk(i, depth * 2)) SPe_table.push_back(i >> depth);
	}
	ROBDD SPe;
	SPe.FromTrueValueVector(SPe_table);
	cout << "\nSPe:" << endl;
	SPe.Print();
	ROBDD V = AND(T, SPe);
	cout << "\nV:" << endl;
	V.Print();
	tn = AND(tn, V);
	return tn;
}

ROBDD EU(Graph G, ROBDD robdd1, ROBDD robdd2)
{ //V = {s ∈ T | ∃t ∈ U : s → t}
	cout << "\nImplementing EU..." << endl;
	int finished = 0;
	ROBDD T = robdd1.CloneROBDD();
	ROBDD u0 = robdd2.CloneROBDD();
	ROBDD un = u0.CloneROBDD();
	int epoch = 0;
	while (!finished)
	{
		cout << "\nEpoch " << epoch << endl;
		ROBDD U = un.CloneROBDD();
		cout << "\nu" << epoch << ":" << endl;
		un.Print();
		vector<int> P1_table;
		int depth = ceil(log2(G.num_nodes));
		for (int i = 0; i < G.num_nodes; i++)
		{
			for (int j = 0; j < G.nodes[i]->next.size(); j++)
			{
				P1_table.push_back((i << depth) + G.nodes[i]->nextidx[j]);
			}
		}
		ROBDD P1, P2 = U.CloneROBDD();
		P1.FromTrueValueVector(P1_table);
		for (int i = 0; i < P2.nodes.size(); i++)
		{
			if (P2.nodes[i]->label >= 0) P2.nodes[i]->label += depth;
		}
		cout << "\nP1:" << endl;
		P1.Print();
		cout << "\nP2:" << endl;
		P2.Print();
		ROBDD P = AND(P1, P2);
		cout << "\nP:" << endl;
		P.Print();
		vector<int> SPe_table;
		for (int i = 0; i < pow(2, depth * 2); i++)
		{
			if (P.Walk(i, depth * 2)) SPe_table.push_back(i >> depth);
		}
		ROBDD SPe;
		SPe.FromTrueValueVector(SPe_table);
		cout << "\nSPe:" << endl;
		SPe.Print();
		cout << "\nT:" << endl;
		SPe.Print();
		ROBDD V = AND(T, SPe);
		cout << "\nV:" << endl;
		V.Print();
		ROBDD last = un.CloneROBDD();
		un = OR(un, V);
		if (Equal(un.root, last.root))
		{
			cout << "\nun=un-1" << endl;
			finished = 1;
		}
		epoch++;
	}
	return un;
}

ROBDDNode * Clone(ROBDDNode* src)
{
	ROBDDNode* ret = new ROBDDNode;
	if (src->label < 0)
	{
		ret->label = src->label;
		ret->value.value = src->value.value;
		return ret;
	}
	ret->label = src->label;
	ret->value.successor.false_branch = Clone(src->value.successor.false_branch);
	ret->value.successor.true_branch = Clone(src->value.successor.true_branch);
	return ret;
}

bool Contain(ROBDDNode * node, int label)
{
	vector<ROBDDNode*> nodes;
	nodes = NodeVector(node);
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->label == label) return true;
	}
	return false;
}
