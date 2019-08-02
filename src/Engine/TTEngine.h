#pragma once

#include "IEngine.h"
#include <stack>

using namespace std;

class TTEngine : public IEngine
{
private:
	int validModels;

	bool modelEval(vector<pair<string, bool>> model, string q);

	bool checkAll(KnowledgeBase& kb, const string& q, stack<string> symbols, vector<pair<string, bool>> model);
public:
	bool ask(KnowledgeBase& kb, const string& q) override;
};

