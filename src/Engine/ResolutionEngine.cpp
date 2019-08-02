#include "ResolutionEngine.h"

bool ResolutionEngine::ask(KnowledgeBase& cnfkb, const string& q) {
	//show satisfiability of q via negation proof of ~q
	//kb + ~q
	cnfkb.tell(TokenUtil::inverse(q));

	vector<Prop> clauses = cnfkb.getPropositions();

	//print some info
	cout << "=====" << endl;
	cout << "KB:" << endl;
	cout << "=====" << endl;
	
	for (int i = 0; i < clauses.size(); i++) {
		cout << "S" << i << ": " << clauses[i].getSignedId() << endl;
	}

	vector<string> symbols = cnfkb.getSignedSymbols();
	cout << "Symbol list: ";
	for (int i = symbols.size() - 1; i >= 0; i--) {
		cout << symbols[i];
		if (i > 0)
			cout << ", ";
		else cout << endl << endl;
	}

	vector<Prop> newClauses;
	vector<Prop> resolvents;

	//hash table to check for duplicate resolvents and prevent exponential explosion of clauses
	map<string, bool> clausesSeen;
	for (Prop& p : clauses) {
		clausesSeen.insert(pair<string, bool>(p.getSignedId(), true));
	}

	if (clauses.empty())
		return false;

	while (true) {
		//pair up all clauses O(nlog(n))
		for (int i = 0; i < clauses.size()-1; i++) {
			for (int j = i+1; j < clauses.size(); j++) {
				resolvents = resolve(clauses[i], clauses[j]);

				//KB exploded, ~q is unsat therefore q is sat
				if (hasEmptyClause(resolvents)) {
					combine(clauses, newClauses);
					printOutput(true, clauses);
					return true;
				}

				removeDuplicates(resolvents, clausesSeen);
				combine(newClauses, resolvents);
			}
		}

		if (isSubset(newClauses, clauses)) {
			combine(clauses, newClauses);
			printOutput(false, clauses);
			return false;
		}

		combine(clauses, newClauses);
		newClauses.clear();
	}
}

void ResolutionEngine::removeDuplicates(vector<Prop>& resolvents, map<string, bool>& clausesSeen) {
	for (int i = resolvents.size() - 1; i >= 0; i--) {
		if (clausesSeen.find(resolvents[i].getSignedId()) != clausesSeen.end())
			resolvents.erase(resolvents.begin() + i);
		else clausesSeen[resolvents[i].getSignedId()];
	}
}

void ResolutionEngine::printOutput(const bool result, const vector<Prop>& clauses) const {
	cout << "clauses resolved/explored: " << endl;
	for (int i = 0; i < clauses.size(); i++) {
		cout << clauses[i].getSignedId();
		if (i < clauses.size() - 1)
			cout << "," << endl;
	}

	cout << endl << endl;

	if (result)
		cout << "YES: ";
	else cout << "NO: ";
}

vector<Prop> ResolutionEngine::resolve(Prop& a, Prop& b) {
	vector<Prop> resolvents = a.resolve(b);
	if (resolvents.size() == 0)
		return resolvents;

	vector<Prop> result;

	for (Prop& p : resolvents) {
		//check for empty clause and shortcut out
		vector<string> symbols = p.getSignedSymbols();
		if (symbols.size() == 0)
			return resolvents;

		//get rid of useless clauses e.g. !b||b
		bool complementary = false;
		for (int i = 0; i < symbols.size()-1; i++) {
			for (int j = 1; j < symbols.size(); j++) {
				if (p.areComplementary(symbols[i], symbols[j])) {
					complementary = true;
					break;
				}
			}
			//short circuit out to next Proposition
			if (complementary)
				break;
		}

		if (!complementary) {
			//remove duplicate symbols to prevent explosion of terms
			TokenUtil::removeDuplicateSymbols(p);
			result.push_back(p);
		}
			
	}

	return result;
}

void ResolutionEngine::combine(vector<Prop>& a, vector<Prop>& b) {
	for (int i = 0; i < b.size(); i++) {
		a.push_back(b[i]);
	}
}

//check if a is subset of b
bool ResolutionEngine::isSubset(vector<Prop> a, vector<Prop> b) {
	//hash table for O(m+n)
	map<string, bool> hashmap;

	for (int i = 0; i < b.size(); i++) {
		hashmap[b[i].getSignedId()] = true;
	}

	for (int i = 0; i < a.size(); i++) {
		if (!hashmap[a[i].getSignedId()])
			return false;
	}

	return true;
}

bool ResolutionEngine::hasEmptyClause(vector<Prop> clauses) {
	for (int i = 0; i < clauses.size(); i++) {
		if (clauses[i].getNumOfSymbols() == 0) {
			return true;
		}
	}

	return false;
}