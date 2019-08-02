#include "TTEngine.h"

using namespace std;

bool TTEngine::ask(KnowledgeBase& kb, const string& q) {
	validModels = 0;

	//get all symbols
	stack<string> symbols = kb.getUnknownSymbols();

	//models
	vector<pair<string, bool>> model;
	bool result = checkAll(kb, q, symbols, model);

	cout << (result ? "YES: " : "NO: ") << validModels << endl;
	
	return result;
}

bool TTEngine::checkAll(KnowledgeBase& kb, const string& q, stack<string> symbols, vector<pair<string, bool>> model) {
	if (symbols.empty()) {
		
		//is model valid for the kb?
		if (kb.eval(model)) {
			//model is valid, evaluate if kb entails q
			validModels++;
			return (modelEval(model, q) || kb.hasProp(q));
		}
		else return true;
	}
	else {
		string p = symbols.top();
		symbols.pop();

		//branch true false
		auto model2 = model;
		model.push_back(pair<string, bool>(p, true));
		model2.push_back(pair<string, bool>(p, false));

		return
			checkAll(kb, q, symbols, model) && checkAll(kb, q, symbols, model2);
	}
}

bool TTEngine::modelEval(vector<pair<string, bool>> model, string q) {
	//guard
	if (q.empty())
		return true;

	for (unsigned int i = 0; i < model.size(); i++) {
		if (model[i].first == TokenUtil::trimNegation(q)) {
			if (q[0] == '~' && !model[i].second)
				return true;
			else if (isalnum(q[0]) && model[i].second)
				return true;
		}
	}

	return false;
}
