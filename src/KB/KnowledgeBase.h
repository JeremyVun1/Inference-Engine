#pragma once
#include <string>
#include <vector>
#include "..\TokenUtil.h"
#include "Prop.h"

using namespace std;

class KnowledgeBase
{
protected:
	vector<string> symbols;
	vector<string> signedSymbols;
	map<string, Prop> propositions;
	
	void registerSymbols(vector<string> tokens);
	void addSymbol(string& s, vector<string>& symbolList);
public:
	KnowledgeBase();
	~KnowledgeBase();

	virtual void tell(string sentence);
	void remove(string sentence);

	virtual bool eval();
	bool eval(vector<pair<string, bool>>& model);
	bool eval(string& symbol);

	//getters
	bool hasProp(string q);
	stack<string> getUnknownSymbols() const;
	vector<string> getSymbols() const;
	vector<string> getSignedSymbols() const;
	vector<Prop> getPropositions() const;
};

