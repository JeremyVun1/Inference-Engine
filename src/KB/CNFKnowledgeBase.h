#pragma once

#include "KnowledgeBase.h"

class CNFKnowledgeBase : public KnowledgeBase
{
private:
	//CNF methods
	vector<string> removeBiDirection(vector<string>& tokens);
	vector<vector<string>> parseClauses(vector<string>& tokens, int begin, int end);

	vector<string> pushNegations(vector<string> tokens);

	vector<vector<string>> convertToCNF(string& sentence);
	vector<string> removeImplication(vector<string>& tokens);

	vector<vector<string>> distributive(vector<string>& tokens);
	vector<string> distribute(vector<string> orTokens, vector<string> andTokens);

	vector<string> getPremises(const vector<string>& tokens);
	vector<string> invertClause(const vector<string>& tokens);
	vector<string> getHead(vector<string>& tokens);
	
	bool hasAnd(vector<string> clause);
	bool hasImplication(const vector<string>& tokens) const;

	//helper methods
	string getClauseSentence(vector<string>& tokens);
	vector<string> implicate(vector<string>& left, vector<string>& right);
public:
	CNFKnowledgeBase();

	void tell(string sentence) override;
	//bool eval() override;
};

