#include "DPLLEngine.h"

#include <iostream>

using namespace std;

/*
Resolution is slow and uses lots of memory.
Use DPLL (DFS approach) to find SAT instead.

DPLL recursion
0. terminal conditions
1. propagate literals
2. if no literals, assert on one and branch true/false
*/

bool DPLLEngine::ask(KnowledgeBase& cnfkb, const string& q) {
	//show satisfiability or unsatisfiability of ~q with (kb + ~q)
	cnfkb.tell(TokenUtil::inverse(q));
	vector<Prop> clauses = cnfkb.getPropositions();
	cout << "=====" << endl;
	cout << "KB:" << endl;
	cout << "=====" << endl;

	for (int i = 0; i<clauses.size(); i++) {
		cout << "S" << i << ": " << clauses[i].getSignedId() << endl;
	}

	//initialise list of unknown symbols for branching and propagation
	vector<string> symbols = cnfkb.getSignedSymbols();
	cout << "Symbol list: ";
	for (int i = symbols.size()-1; i >= 0; i--) {
		cout << symbols[i];
		if (i > 0 )
			cout << ", ";
		else cout << endl << endl;

		if (symbols[i] == TokenUtil::inverse(q)) {
			symbols.erase(symbols.begin() + i);
		}
	}

	//start recursion with ~q literal
	Prop p = getClauseWithId(clauses, TokenUtil::inverse(q));
	bool result = dpll(clauses, p, symbols);

	printOutput(result);

	cnfkb.remove(TokenUtil::inverse(q));

	return result;
}

bool DPLLEngine::dpll(vector<Prop> clauses, Prop l, vector<string> symbols) {
	//TERMINALS
	//all clauses evaluated to true and eliminated
	if (clauses.size() == 0)
		return true;
	//empty set found. kb & ~q is not SAT
	if (hasEmptyClause(clauses))
		return false;

	output.push_back(l.getSignedId());

	propagate(l, clauses);

	//find another literal to propagate
	for (int i = 0; i < clauses.size(); i++) {
		if (clauses[i].getIsUnitClause())
			continue;

		//literal found, update symbols list
		for (int j = 0; j < symbols.size(); j++) {
			if (symbols[j] == clauses[i].getSignedId()) {
				symbols.erase(symbols.begin() + j);
				break;
			}
		}

		bool result = dpll(clauses, clauses[i], symbols);

		//update path route
		if (result)
			output.erase(output.end() - 1);

		return result;
	}

	//literal not found. Branch (true/false) on a symbol
	if (clauses.size() > 0 && symbols.size() > 0) {
		vector<string> tokens;

		//true branch
		tokens.push_back(symbols[0]);
		Prop pTrue(tokens, symbols[0]);
		//false branch
		tokens.push_back("~");
		Prop pFalse(tokens, (symbols[0] + "~"));

		symbols.erase(symbols.begin());

		//branch
		bool result = (dpll(clauses, pTrue, symbols) || dpll(clauses, pFalse, symbols));

		//track path
		if (result)
			output.erase(output.end() - 1);

		return result;
	}

	return dpll(clauses, l, symbols);
}

bool DPLLEngine::propagate(Prop l, vector<Prop>& clauses) {
	bool result = false;

	//propagate the literal throughout all our clauses
	for (int i = clauses.size() - 1; i >= 0; i--) {
		//if the clause returns that it has been completely inferred, then erase it
		if (clauses[i].propagate(l)) {
			clauses.erase(clauses.begin() + i);
			result = true;
		}
	}

	return result;
}

bool DPLLEngine::hasEmptyClause(vector<Prop> clauses) {
	for (int i = 0; i < clauses.size(); i++) {
		if (clauses[i].getNumOfSymbols() == 0) {
			return true;
		}
	}

	return false;
}

Prop DPLLEngine::getClauseWithId(const vector<Prop>& clauses, const string& q) const {
	for (const Prop& p : clauses) {
		if (p.getSignedId() == q)
			return p;
	}

	throw "Cannot find clause with specified id";
}



void DPLLEngine::printOutput(const bool result) const {
	//print inverse because we are checking for unsatisfiability of q
	for (int i = 0; i < output.size(); i++) {
		cout << output[i];
		if (i < output.size() - 1)
			cout << ", ";
	}
	cout << endl;

	if (result)
		cout << endl << endl << "NO: ";
	else cout << "YES: ";
}
