#pragma once

#include "IEngine.h"
#include "PropCount.h"
#include "..\KB\Prop.h"

class FCEngine : public IEngine
{
private:
	vector<string> output;

	void tryTellFact(KnowledgeBase& kb, string& s);

	stack<string> getSignedKnownFacts(KnowledgeBase& kb);

	vector<PropCount> InitPropCounter(KnowledgeBase& kb);

	map<string, bool> InitInferredList(KnowledgeBase& kb);

	void printOutput(bool result);
public:
	FCEngine();
	
	bool ask(KnowledgeBase& kb, const string& q) override;
};
