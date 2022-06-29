#include <iostream>
#include <string>
#include <map>
#include "ROBDD.h"

using namespace std;
ROBDD parse(string expression);
map<string, int> sym_to_graph;
map<int, string> graph_to_sym;
vector<ROBDD> robdds;
Graph total_graph;
int main()
{
	int n;
	cout << "Input number of symbols:";
	cin >> n;
	cout << "Input these symbols,separated by blank:\n";
	for (int i = 0; i < n; i++)
	{
		string sym;
		cin >> sym;
		sym_to_graph[sym] = i;
		graph_to_sym[i] = sym;
	}
	int num_vert, num_edge;
	cout << "Input total number of vertices:";
	cin >> num_vert;
	cout << "Input total number of edges:";
	cin >> num_edge;
	for (int i = 0; i < num_vert; i++)
	{
		total_graph.AddNode(0);
	}
	cout << "Input the source node and destination node of each edge respectively:" << endl;
	for (int i = 0; i < num_edge; i++)
	{
		int src, dst;
		cin >> src >> dst;
		total_graph.AddEdge(src, dst);
	}
	for (int i = 0; i < n; i++)
	{
		cout << "Input true vertices for symbol " << graph_to_sym[i] << ", -1 indicates end";
		int vert;
		vector<int> table;
		while(1)
		{
			cin >> vert;
			if (vert == -1) break;
			table.push_back(vert);
		}
		ROBDD robdd;
		robdd.FromTrueValueVector(table);
		robdds.push_back(robdd);
	}
	while (1)
	{
		cout << "Input your expression(input exit to quit):\n";
		string expression;
		cin >> expression;
		if (expression == "exit") break;
		ROBDD result = parse(expression);
		cout << "\nResult:" << endl;
		result.Print();
	}
	return 0;
}
ROBDD parse(string expression)
{
	cout << "\nComputing " << expression << "..." << endl;
	if (!expression.empty())
	{
		int index = 0;
		while ((index = expression.find(' ', index)) != string::npos)
		{
			expression.erase(index, 1);
		}
	}
	int pos = expression.find('(', 0);
	if (pos == string::npos)
	{
		return robdds[sym_to_graph[expression]];
	}
	string op = expression.substr(0, pos);
	if (op == "") return parse(expression.substr(1, expression.length() - 2));
	if (op == "and" || op == "AND")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		int comma = remainder.find(',', 0);
		string expr1 = remainder.substr(0, comma);
		string expr2 = remainder.substr(comma, remainder.length() - comma);
		return AND(parse(expr1), parse(expr2));
	}
	else if (op == "or" || op == "OR")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		int comma = remainder.find(',', 0);
		string expr1 = remainder.substr(0, comma);
		string expr2 = remainder.substr(comma, remainder.length() - comma);
		return OR(parse(expr1), parse(expr2));
	}
	else if (op == "imply" || op == "IMPLY")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		int comma = remainder.find(',', 0);
		string expr1 = remainder.substr(0, comma);
		string expr2 = remainder.substr(comma, remainder.length() - comma);
		return IMPLY(parse(expr1), parse(expr2));
	}
	else if (op == "ex" || op == "EX")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		return EX(total_graph, parse(remainder));
	}
	else if (op == "eg" || op == "EG")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		return EG(total_graph, parse(remainder));
	}
	else if (op == "eu" || op == "EU")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		int comma = remainder.find(',', 0);
		string expr1 = remainder.substr(0, comma);
		string expr2 = remainder.substr(comma, remainder.length() - comma);
		return EU(total_graph, parse(expr1), parse(expr2));
	}
	else if (op == "not" || op == "NOT")
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		return NOT(parse(remainder));
	}
	else if (op == "af" || op == "AF") //AF p=~EG~p
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		cout << "AF(" << remainder << ")=NOT(EG(NOT" << remainder << ")))" << endl;
		return NOT(EG(total_graph,NOT(parse(remainder))));
	}
	else if (op == "ax" || op == "AX") //AX p=~EX~p
	{
		string remainder = expression.substr(pos, expression.length() - pos);
		cout << "AX(" << remainder << ")=NOT(EX(NOT" << remainder << ")))" << endl;
		return NOT(EX(total_graph, NOT(parse(remainder))));
	}
	else if (op == "ef" || op == "EF") //EF ϕ ≡ E[⊤ U ϕ]
	{
		ROBDD robdd_true;
		ROBDDNode* NewNode = new ROBDDNode;
		NewNode->label = -1;
		NewNode->value.value = 0;
		robdd_true.nodes.push_back(NewNode);
		robdd_true.root = NewNode;
		string remainder = expression.substr(pos, expression.length() - pos);
		cout << "EF(" << remainder << ")=E(⊤ U " << remainder << ")" << endl;
		return EU(total_graph, robdd_true, parse(remainder));
	}
	else if (op == "ag" || op == "AG") //AG ϕ ≡ ~E[⊤ U ~ϕ]
	{
		ROBDD robdd_true;
		ROBDDNode* NewNode = new ROBDDNode;
		NewNode->label = -1;
		NewNode->value.value = 0;
		robdd_true.nodes.push_back(NewNode);
		robdd_true.root = NewNode;
		string remainder = expression.substr(pos, expression.length() - pos);
		cout << "AG(" << remainder << ")=NOT(E(⊤ U NOT(" << remainder << ")))" << endl;
		return NOT(EU(total_graph, robdd_true, NOT(parse(remainder))));
	}
}