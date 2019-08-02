#pragma once

#include "IEngine.h"
#include "..\KB\KnowledgeBase.h"

#include <map>
#include <string>
#include <vector>

class BCEngine : public IEngine
{
protected:
	vector<string> output;
	vector<Prop> whereSymbolIsHead(string q, vector<Prop>& propositions);
	void clearStack(stack<bool>& s);

	void printOutput(const bool result) const;

	bool evalSignedSymbol(string q, vector<Prop>& kbPropositions);
public:
	BCEngine();

	virtual bool ask(KnowledgeBase& kb, const std::string& q) override;
};

