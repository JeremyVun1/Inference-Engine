#include <string>
#include <iostream>
#include <cctype>

#include "KnowledgeBase.h"

using namespace std;

// PRECEDENCE
// ~, &, ||, =>, <=>
KnowledgeBase::KnowledgeBase() {}

KnowledgeBase::~KnowledgeBase() {}

void KnowledgeBase::tell(string sentence) {
	//split sentence into tokens
	sentence = TokenUtil::trimspaces(sentence);
	vector<string> tokens = TokenUtil::extractTokens(sentence);
	
	//register our symbols
	registerSymbols(tokens);

	//remove bidirections
	vector<vector<string>> clauses = TokenUtil::removeBiDirection(tokens);

	for (int i = 0; i < clauses.size(); i++) {
		string id = TokenUtil::getString(clauses[i]);

		//convert to reverse polish notation
		vector<string> clause = TokenUtil::infixToRpn(clauses[i]);

		//create proposition and store in map
		Prop p(clause, propositions, id);
		propositions.insert(pair<string, Prop>(p.getId(), p));
	}
}

//evaluate the KB as is
bool KnowledgeBase::eval() {
	for (auto ite = propositions.begin(); ite != propositions.end(); ++ite) {
		if (ite->second.getIsUnitClause() && !ite->second.eval())
			return false;
	}

	return true;
}

//evaluate a model
bool KnowledgeBase::eval(vector<pair<string, bool>>& model) {
	for (unsigned int i = 0; i < model.size(); i++) {
		if (model[i].second)
			tell(model[i].first);
		else {
			string tok = "~" + model[i].first;
			tell(tok);
		}
	}
	bool result = eval();

	for (unsigned int i = 0; i < model.size(); i++) {
		propositions.erase(model[i].first);
	}

	return result;
}

//evaluate a symbol (literal proposition)
bool KnowledgeBase::eval(string& symbol) {
	for (auto p : propositions) {
		if (p.first == symbol)
			return p.second.eval();
	}

	cerr << symbol << "symbol not found in kb" << endl;
	return false;
}

//input is in infix form
void KnowledgeBase::registerSymbols(vector<string> tokens) {
	for (unsigned int i = 0; i < tokens.size(); i++) {
		if (isalnum(tokens[i][0])) {
			addSymbol(tokens[i], symbols);

			string signedSymbol = "";
			if (i > 0 && tokens[i-1] == "~")
				signedSymbol += tokens[i-1];
			signedSymbol += tokens[i];

			addSymbol(signedSymbol, signedSymbols);
		}
	}
}

void KnowledgeBase::addSymbol(string& s, vector<string>& symbolList) {
	//check if we already have the symbol
	for (unsigned int i = 0; i < symbolList.size(); i++) {
		if (symbolList[i] == s)
			return;
	}

	symbolList.push_back(s);
}

bool KnowledgeBase::hasProp(string q) {
	Prop p(TokenUtil::extractTokens(q), propositions, q);

	auto ite = propositions.find(p.getId());
	if (ite != propositions.end())
		return ite->second.getValue();
	else return false;
}

void KnowledgeBase::remove(string sentence) {
	propositions.erase(sentence);
}

/////////
//GETTERS
/////////
vector<Prop> KnowledgeBase::getPropositions() const {
	vector<Prop> result;

	for (pair<string, Prop> p : propositions) {
		result.push_back(p.second);
	}

	return result;
}

vector<string> KnowledgeBase::getSymbols() const {
	return symbols;
}

vector<string> KnowledgeBase::getSignedSymbols() const {
	return signedSymbols;
}

//return all the symbols that aren't found as literal propositions
//reduce number of models that need to be created
stack<string> KnowledgeBase::getUnknownSymbols() const {
	stack<string> result;

	for (unsigned int i = 0; i < symbols.size(); i++) {
		if (propositions.find(symbols[i]) == propositions.end())
			result.push(symbols[i]);
	}

	return result;
}
