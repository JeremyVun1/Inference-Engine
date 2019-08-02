#include "FCEngine.h"
#include <map>

using namespace std;

FCEngine::FCEngine() {}

bool FCEngine::ask(KnowledgeBase& kb, const string& q) {
	//build prop counter to track how many premises we know of a prop
	vector<PropCount> count = InitPropCounter(kb);
	map<string, bool> inferred = InitInferredList(kb);

	//seed with known facts
	stack<string> agenda = getSignedKnownFacts(kb);

	bool result = false;
	while (!agenda.empty()) {
		string top = agenda.top();
		agenda.pop();

		output.push_back(top);

		//fact found
		if (top == q) {
			result = true;
			break;
		}
			
		//already explored
		if (inferred[top])
			continue;
		
		//symbol not explored
		inferred[top] = true;
		for (PropCount& pc : count) {
			if (pc.symbolInPremises(top)) {
				pc.count--;

				//forward chaining count check
				if (pc.count == 0)
					agenda.push(pc.getSignedHead());
			}
		}
	}

	printOutput(result);
	return result;
}

stack<string> FCEngine::getSignedKnownFacts(KnowledgeBase& kb) {
	stack<string> result;

	vector<Prop> propositions = kb.getPropositions();
	for (Prop p : propositions) {
		if (!p.getIsUnitClause()) {
			result.push(p.getSignedId());
		}
	}

	return result;
}

vector<PropCount> FCEngine::InitPropCounter(KnowledgeBase& kb) {
	vector<PropCount> result;

	vector<Prop> propositions = kb.getPropositions();

	for (Prop p : propositions) {
		if (p.getIsUnitClause()) {
			vector<string> premiseTokens = p.getSignedPremises();
			PropCount propCount(p, (int)premiseTokens.size());

			result.push_back(propCount);
		}
	}

	return result;
}

map<string, bool> FCEngine::InitInferredList(KnowledgeBase& kb) {
	map<string, bool> result;

	for (string s : kb.getSignedSymbols())
		result.insert(pair<string, bool>(s, false));

	return result;
}

void FCEngine::printOutput(bool result) {
	if (result)
		cout << "YES: ";
	else cout << "NO: ";

	//print trace
	for (int i = 0; i < output.size(); i++) {
		cout << output[i];
		if (!(i == output.size() - 1))
			cout << ", ";
	}
}
