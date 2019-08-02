#pragma once

#include "IEngine.h"
#include "..\KB\CNFKnowledgeBase.h"

class DPLLEngine : public IEngine
{
private:
	vector<string> output;

	bool dpll(vector<Prop> clauses, Prop l, vector<string> symbols);
	bool propagate(Prop l, vector<Prop>& clauses);

	bool hasEmptyClause(vector<Prop> clauses);
	Prop getClauseWithId(const vector<Prop>& clauses, const string& q) const;

	void printOutput(const bool result) const;
public:
	bool ask(KnowledgeBase& cnfkb, const string& q) override;
};







