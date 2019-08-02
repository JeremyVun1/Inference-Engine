#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>

using namespace std;

class Prop
{
private:
	vector<string> tokens;
	map<string, Prop>* propositions;

	bool val, isUnitClause;
	string id;

	void negEval(stack<bool>& s);
	void andEval(stack<bool>& s);
	void orEval(stack<bool>& s);
	void impEval(stack<bool>& s);
	void biEval(stack<bool>& s);

	vector<string> getPremiseSymbols() const;

	//resolution helper methods
	vector<string> tokensExcept(vector<string> tokens, string signedSymbol) const;
	vector<string> combineResolvedTokens(vector<string> aTokens, vector<string> bTokens) const;
public:
	Prop(vector<string> tokens, string sentence);
	Prop(vector<string> tokens, map<string, Prop>& propositions, string sentence);
	
	bool eval();
	bool evalPremises() const;
	vector<Prop> resolve(Prop& other) const;
	bool propagate(const Prop& p);

	bool symbolInHead(const string& s) const;

	bool operator==(const Prop& other) const;
	bool areComplementary(const string& a, const string& b) const;

	//getters
	string getId() const;
	string getSignedId() const;
	bool getIsUnitClause() const;
	bool getValue() const;
	vector<string> getSignedPremises() const;
	vector<string> getSignedPremiseStructure() const;
	string getHead() const;
	string getSignedHead() const;
	int getNumOfSymbols() const;
	vector<string> getSignedSymbols() const;
	vector<string> getTokens() const;

	//setters
	void setTokens(vector<string> tokens);
};

