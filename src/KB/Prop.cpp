#include "Prop.h"

#include <iostream>
#include <cctype>
#include "..\TokenUtil.h"

using namespace std;

Prop::Prop(vector<string> tokens, string sentence)
	: tokens(tokens), propositions(), id(sentence), isUnitClause(true) {

	if (tokens.size() == 1) {
		val = true;
		isUnitClause = false;
	}
	else if (tokens.size() == 2 && tokens[1] == "~") {
		id = tokens[0];
		val = false;
		isUnitClause = false;
	}
}

Prop::Prop(vector<string> tokens, map<string, Prop>& propositions, string sentence)
	: tokens(tokens), propositions(&propositions), id(sentence), isUnitClause(true) {

	if (tokens.size() == 1) {
		val = true;
		isUnitClause = false;
	}
	else if (tokens.size() == 2 && tokens[1] == "~") {
		id = tokens[0];
		val = false;
		isUnitClause = false;
	}
}

bool Prop::eval() {
	//shortcut for positive literals
	if (tokens.size() == 1 || tokens.size() == 2 && tokens[1] == "~")
		return val;

	stack<bool> s;

	//unwind rpn form and evaluate using other known facts
	for (unsigned int i = 0; i < tokens.size(); i++) {
		if (isalnum(tokens[i][0])) {
			auto ite = propositions->find(tokens[i]);
			if (ite == propositions->end()) {
				std::cerr << "cannot find prop " << tokens[i] << " to evaluate" << std::endl;
				s.push(false);
			}
			else {
				s.push(ite->second.eval());
			}
		}
		
		//pull necessary tokens from stack to perform operation
		else if (tokens[i] == "~")
			negEval(s);
		else if (tokens[i] == "&")
			andEval(s);
		else if (tokens[i] == "||")
			orEval(s);
		else if (tokens[i] == "=>")
			impEval(s);
		else if (tokens[i] == "<=>")
			biEval(s);
	}

	return s.top();
}

void Prop::negEval(stack<bool>& s) {
	bool a = s.top();
	s.pop();
	s.push(!a);
}

void Prop::andEval(stack<bool>& s) {
	bool b = s.top();
	s.pop();
	bool a = s.top();
	s.pop();

	s.push(a&b);
}

void Prop::orEval(stack<bool>& s) {
	bool b = s.top();
	s.pop();
	bool a = s.top();
	s.pop();

	s.push(a || b);
}

void Prop::impEval(stack<bool>& s) {
	bool b = s.top();
	s.pop();
	bool a = s.top();
	s.pop();

	s.push(!a || b);
}

void Prop::biEval(stack<bool>& s) {
	bool b = s.top();
	s.pop();
	bool a = s.top();
	s.pop();

	s.push(a == b);
}

vector<string> Prop::getSignedSymbols() const {
	vector<string> result;

	//tokens should be in rpn. negation is exactly after the symbol if it exists
	for (int i=0; i<tokens.size(); i++) {
		if (isalnum(tokens[i][0])) {
			string signedSymbol = "";
			signedSymbol += tokens[i];

			if (i + 1 < tokens.size() && tokens[i + 1] == "~")
				signedSymbol = tokens[i + 1] + signedSymbol;

			result.push_back(signedSymbol);
		}
	}

	return result;
}

vector<Prop> Prop::resolve(Prop& other) const {
	vector<Prop> result;
	auto aSignedSymbols = getSignedSymbols();
	auto bSignedSymbols = other.getSignedSymbols();

	for (int i = 0; i < aSignedSymbols.size(); i++) {
		string a = aSignedSymbols[i];
		for (int j = 0; j < bSignedSymbols.size(); j++) {
			string b = bSignedSymbols[j];

			if (areComplementary(a, b)) {
				vector<string> newTokens = combineResolvedTokens(tokensExcept(tokens, a), tokensExcept(other.tokens, b));
				string id = TokenUtil::cnfTokensToString(newTokens);
				newTokens = TokenUtil::infixToRpn(newTokens);

				Prop p(newTokens, id);
				result.push_back(p);
			}
		}
	}

	return result;
}

//note: tokens are in rpn
vector<string> Prop::tokensExcept(vector<string> tokens, string signedSymbol) const {
	string symbol = TokenUtil::trimNegation(signedSymbol);
	
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == symbol) {
			//erase token symbol
			auto ite = tokens.begin() + i;
			tokens.erase(tokens.begin() + i);

			if (tokens.size() == 0)
				break;

			//erase token negation
			ite = tokens.begin() + i;
			if ((*ite) == "~") {
				tokens.erase(ite);

				if (tokens.size() == 0)
					break;
			}

			//remove the next "||" we can find
			bool orFound = false;
			for (auto tokenIte = (tokens.begin() + i); tokenIte != tokens.end(); tokenIte++) {
				if ((*tokenIte) == "||") {
					tokens.erase(tokenIte);
					orFound = true;
					break;
				}
			}

			//if we can't find a "||", then try to remove the most previous "||"
			if (!orFound) {
				for (auto tokenIte = tokens.begin() + i; ite != tokens.end(); tokenIte--) {
					if ((*tokenIte) == "||") {
						tokens.erase(tokenIte);
						orFound = true;
						break;
					}
				}
			}
		}
	}

	return tokens;
}

vector<string> Prop::combineResolvedTokens(vector<string> aTokens, vector<string> bTokens) const {
	//convert to infix to make anding the tokens easier
	aTokens = TokenUtil::rpnToInfix(aTokens);
	bTokens = TokenUtil::rpnToInfix(bTokens);

	vector<string> result;

	//add tokens from first string
	if (aTokens.size() > 0) {
		for (int i = 0; i < aTokens.size(); i++)
			result.push_back(aTokens[i]);
	}

	//OR the two strings together
	if (bTokens.size() > 0 && aTokens.size() > 0)
		result.push_back("||");

	//add tokens from second string
	if (bTokens.size() > 0) {
		for (int i = 0; i < bTokens.size(); i++)
			result.push_back(bTokens[i]);
	}

	return result;
}

bool Prop::areComplementary(const string& a, const string& b) const {
	string aSymbol = TokenUtil::trimNegation(a);
	string bSymbol = TokenUtil::trimNegation(b);

	return (aSymbol == bSymbol) && (a[0] == '~') != (b[0] == '~');
}

string Prop::getId() const {
	return id;
}

string Prop::getSignedId() const {
	if (isUnitClause || val)
		return id;
	else
		return ("~" + id);
}

bool Prop::getIsUnitClause() const {
	return isUnitClause;
}

bool Prop::getValue() const {
	return val;
}

//e.g. ["~a", "b", "c", "~d"]
vector<string> Prop::getSignedPremises() const {
	//shortcut for literals
	if (tokens.size() == 1)
		return tokens;

	vector<string> result;

	//get infix notation and return premises up to =>
	bool negation = false;
	for (string s : TokenUtil::rpnToInfix(tokens)) {
		if (s == "=>" || s == "<=>")
			break;
		else if (s == "~")
			negation = true;
		else if (isalnum(s[0])) {
			s = (negation ? "~" : "") + s;
			result.push_back(s);
			negation = false;
		}
	}

	return result;
}

//e.g. ["~a", "||", "b", "&", "~c]
vector<string> Prop::getSignedPremiseStructure() const {
	vector<string> result;

	bool negated = false;
	vector<string> infixTokens = TokenUtil::rpnToInfix(tokens);
	for (string p : infixTokens) {
		if (p == "=>" || p == "<=>")
			break;
		else if (p == "~")
			negated = true;
		else if (isalnum(p[0])) {
			string x = (negated ? "~" : "") + p;
			result.push_back(x);
			negated = false;
		}
		
	}
	
	return result;
}

int Prop::getNumOfSymbols() const {
	int result = 0;

	for (string p : tokens) {
		if (isalnum(p[0]))
			result++;
	}

	return result;
}

bool Prop::symbolInHead(const string& s) const {
	return (getHead() == s);
}

bool Prop::propagate(const Prop& pLiteral) {
	vector<string> infixTokens = TokenUtil::rpnToInfix(tokens);

	for (int i = 0; i < infixTokens.size(); i++) {
		//only consider symbol tokens
		if (!isalnum(infixTokens[i][0]))
			continue;

		//signed token string for comparison
		string signedToken = "";
		if (i > 0 && infixTokens[i - 1] == "~")
			signedToken += "~";
		signedToken += infixTokens[i];

		//tokens are the exactly the same, erase entire clause
		if (pLiteral.getSignedId() == signedToken)
			return true;

		//if pLiteral and token are complementary, then remove token from this prop
		else if (areComplementary(pLiteral.getSignedId(), signedToken)) {
			//erase the symbol token
			infixTokens.erase(infixTokens.begin() + i);

			//remove any negation
			if (i > 0 && infixTokens[i - 1] == "~") {
				infixTokens.erase(infixTokens.begin() + i - 1);
				i -= 1;
			}

			//remove || symbol
			//right scan
			bool orRemoved = false;
			if (i < infixTokens.size()) {
				for (int j = i; j < infixTokens.size(); j++) {
					if (infixTokens[j] == "||") {
						infixTokens.erase(infixTokens.begin() + j);
						orRemoved = true;
						break;
					}
				}
			}
			//left scan
			if (!orRemoved && i > 0) {
				for (int j = i-1; j < infixTokens.size(); j++) {
					if (infixTokens[j] == "||") {
						infixTokens.erase(infixTokens.begin() + j);
						break;
					}
				}
			}
		}
	}

	//convert back to rpn
	tokens = TokenUtil::infixToRpn(infixTokens);

	//update clause if it is now a literal
	if (getNumOfSymbols() == 1 && isUnitClause) {
		isUnitClause = false;

		for (string& s : tokens) {
			if (isalnum(s[0])) {
				id = s;
				break;
			}
		}

		val = tokens.size() == 1 ? true : false; //negative or positive literal value
	}

	return false;
}

vector<string> Prop::getPremiseSymbols() const {
	vector<string> result;

	for (string p : TokenUtil::rpnToInfix(tokens)) {
		if (p == "=>" || p == "<=>")
			break;

		result.push_back(p);
	}

	return result;
}

bool Prop::evalPremises() const {
	//shortcut for literals
	if (tokens.size() == 1)
		return val;

	//convert to rpn for evaluation
	auto premiseTokens = TokenUtil::infixToRpn(getPremiseSymbols());
	
	bool negation = false;
	if (premiseTokens.size() == 2 && premiseTokens[1] == "~")
		negation = true;

	//create new proposition and evaluate it against current knowledge base
	Prop temp(premiseTokens, *propositions, "");
	return (negation ? !temp.eval() : temp.eval());
}

string Prop::getSignedHead() const {
	//shortcut for literals
	if (tokens.size() == 1)
		return tokens[0];
	else if (tokens.size() == 2 && tokens[1] == "~")
		return ("~" + tokens[0]);

	string result;
	bool negation = false;
	bool headStart = false;

	//convert to infix and return head symbol after the =>
	vector<string> infixTokens = TokenUtil::rpnToInfix(tokens);
	for (int i = 0; i < infixTokens.size(); i++) {
		if (infixTokens[i] == "=>" || infixTokens[i] == "<=>")
			headStart = true;
		else if (headStart && infixTokens[i] == "~")
			negation = true;
		else if (headStart && isalnum(infixTokens[i][0])) {
			result += ((negation ? "~" : "") + infixTokens[i]);
			negation = false;
		}
	}

	return result;
}

string Prop::getHead() const {
	//convert to infix and return head symbol after the =>
	vector<string> infixTokens = TokenUtil::rpnToInfix(tokens);
	for (int i = infixTokens.size() - 1; i >= 0; i--) {
		if (isalnum(infixTokens[i][0]))
			return infixTokens[i];
		else if (infixTokens[i] == "=>" || infixTokens[i] == "<=>")
			break;
	}

	cerr << "Malformed proposition. No head found" << endl;
	return "";
}

bool Prop::operator==(const Prop& other) const {
	vector<string> aSymbols = getSignedSymbols();
	vector<string> bSymbols = other.getSignedSymbols();

	//shortcut if different number of symbols
	if (aSymbols.size() != bSymbols.size())
		return false;

	//check that every symbol in a is found in b
	for (string& a : aSymbols) {
		bool found = false;

		for (string& b : bSymbols) {
			if (a == b) {
				found = true;
				break;
			}
			else found = false;
		}

		if (!found)
			return false;
	}

	return true;
}

vector<string> Prop::getTokens() const {
	return tokens;
}

//setters
void Prop::setTokens(vector<string> tokens) {
	this->tokens = tokens;
}
