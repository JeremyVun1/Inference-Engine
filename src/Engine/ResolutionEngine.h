#pragma once

#include "IEngine.h"
#include "..\KB\Prop.h"

class ResolutionEngine : public IEngine
{
private:
	vector<Prop> resolve(Prop& a, Prop& b);
	void combine(vector<Prop>& a, vector<Prop>& b);
	
	bool isSubset(vector<Prop> a, vector<Prop> b);
	bool hasEmptyClause(vector<Prop> clauses);

	void removeDuplicates(vector<Prop>& resolvents, map<string, bool>& clausesSeen);

	void printOutput(const bool result, const vector<Prop>& clauses) const;

public:
	bool ask(KnowledgeBase& cnfkb, const string& q) override;
};

