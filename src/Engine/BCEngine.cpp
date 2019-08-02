#include "BCEngine.h"

using namespace std;

BCEngine::BCEngine() {}

bool BCEngine::ask(KnowledgeBase& kb, const string& q) {
	vector<Prop> kbPropositions = kb.getPropositions();

	//if q is true, then all it's premises must also be true
	bool result = evalSignedSymbol(q, kbPropositions);
	output.push_back(q);

	printOutput(result);
	return result;
}

bool BCEngine::evalSignedSymbol(string q, vector<Prop>& kbPropositions) {
	vector<Prop> whereQIsHead = whereSymbolIsHead(q, kbPropositions);
	stack<bool> stackResult;

	for (Prop& p : whereQIsHead) {
		clearStack(stackResult);

		//reached a literal
		if (!p.getIsUnitClause()) {
			return (p.getSignedId() == q);
		}

		//get premise structure of p
		//convert to rpn to make evaluation easier
		vector<string> premiseStructure = p.getSignedPremiseStructure();
		premiseStructure = TokenUtil::infixToRpn(premiseStructure);

		for (string& s : premiseStructure) {
			if (TokenUtil::isSymbol(s)) {
				if (evalSignedSymbol(s, kbPropositions)) {
					output.push_back(s);
					stackResult.push(true);
				}
				else stackResult.push(false);
			}

			//EVALUATE PREMISE TRUTH IF THERE ARE MULTIPLE SYMBOLS IN THE PREMISE
			else if (s == "&") {
				bool a = stackResult.top();
				stackResult.pop();
				bool b = stackResult.top();
				stackResult.pop();

				stackResult.push(a&b);
			}
			else if (s == "||") {
				bool a = stackResult.top();
				stackResult.pop();
				bool b = stackResult.top();
				stackResult.pop();

				stackResult.push(a||b);
			}
		}

		//return if we find that the proposition evaluates to true
		if (stackResult.top())
			return true;
	}

	//propositions exhausted and none evaluated to true
	return false;
}

vector<Prop> BCEngine::whereSymbolIsHead(string q, vector<Prop>& propositions) {
	vector<Prop> result;

	//trim negation to make it easier to search for relevant propositions
	q = TokenUtil::trimNegation(q);

	for (Prop& p : propositions) {
		if (p.getHead() == q)
			result.push_back(p);
	}

	return result;
}

void BCEngine::printOutput(const bool result) const {
	if (result)
		cout << "YES: ";
	else cout << "NO: ";

	//print trace
	for (int i = 0; i < output.size(); i++) {
		cout << output[i];
		if (i < output.size()-1)
			cout << ", ";
	}
}

void BCEngine::clearStack(stack<bool>& s) {
	while (!s.empty())
		s.pop();
}
