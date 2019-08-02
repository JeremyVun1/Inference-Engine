#include "TokenUtil.h"

#include <list>
#include <iostream>
#include <algorithm>

using namespace std;

vector<string> TokenUtil::infixToRpn(const vector<string>& tokens) {
	//shortcut out
	if (tokens.size() <= 1)
		return tokens;

	stack<string> stack;
	vector<string> rpnTok;

	for (unsigned int i = 0; i < tokens.size(); i++) {
		if (isSymbol(tokens[i])) //push operands
		{
			rpnTok.push_back(tokens[i]);
		}
		else if (tokens[i] == "(") //push open bracket
		{
			stack.push(tokens[i]);
		}
		else if (tokens[i] == ")") //dump stack until we reach an open bracket
		{
			while (!stack.empty() && stack.top() != "(") {
				rpnTok.push_back(stack.top());
				stack.pop();
			}

			if (!stack.empty() && stack.top() == "(") //pop opening bracket
				stack.pop();
		}
		else if (stack.empty() || opPriority(tokens[i]) > opPriority(stack.top())) //push operator token onto the stack if stack is empty or it has higher priority than the last stack item
		{
			stack.push(tokens[i]);
		}
		else //empty operator stack until stack token has lower priority than the token we want to push
		{
			while (!stack.empty() && opPriority(tokens[i]) <= opPriority(stack.top())) {
				rpnTok.push_back(stack.top());
				stack.pop();
			}
			stack.push(tokens[i]);
		}
	}

	//flush stack
	while (!stack.empty()) {
		rpnTok.push_back(stack.top());
		stack.pop();
	}

	return rpnTok;
}

vector<string> TokenUtil::rpnToInfix(const vector<string>& tokens) {
	//shortcut
	if (tokens.size() <= 1)
		return tokens;

	//post fix to infix with minimum brackets
	stack<string> s;
	stack<int> ops;

	for (int i = 0; i < tokens.size(); i++) {
		if (isSymbol(tokens[i])) {
			s.push(tokens[i]);
			ops.push(99);
		}
		else if (tokens[i] == "~") {
			string a = s.top();
			s.pop();
			s.push("~" + a);
		}
		else if (tokens[i] == "&") {
			string op = "&";
			s.push(operate(op, s, ops));
		}
		else if (tokens[i] == "||") {
			string op = "||";
			s.push(operate(op, s, ops));
		}
		else if (tokens[i] == "=>") {
			string op = "=>";
			s.push(operate(op, s, ops));
		}
		else if (tokens[i] == "<=>") {
			string op = "<=>";
			s.push(operate(op, s, ops));
		}
	}

	return extractTokens(s.top());
}

string TokenUtil::operate(string& op, stack<string>& s, stack<int>& ops) {
	string a = s.top();
	int aOp = ops.top();
	s.pop();
	ops.pop();

	string b = s.top();
	int bOp = ops.top();
	s.pop();
	ops.pop();

	int highestOp = aOp >= bOp ? aOp : bOp;

	//do we need brackets?
	if (opPriority(op) > aOp) {
		//bracket a
		a = "(" + a + ")";
	}
	if (opPriority(op) > bOp) {
		b = "(" + b + ")";
	}

	int lowestOp = highestOp <= opPriority(op) ? highestOp : opPriority(op);
	ops.push(lowestOp);

	return (b + op + a);
}

vector<string> TokenUtil::extractTokens(const string& s) {
	vector<string> result;

	//get the starting index of each token. Symbols might be multiple characters
	vector<int> tokInd;
	bool startSymbol = isSymbol(s);
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] == '~') {
			tokInd.push_back(i);
			startSymbol = true;
		}
		else if (s[i] == '(') {
			tokInd.push_back(i);
			startSymbol = true;
		}
		else if (s[i] == ')') {
			tokInd.push_back(i);
			startSymbol = false;
		}
		else if (!isalnum(s[i]) && !startSymbol) {
			tokInd.push_back(i);
			startSymbol = true;
		}
		else if (isalnum(s[i]) && startSymbol) {
			tokInd.push_back(i);
			startSymbol = false;
		}
	}

	if (tokInd.size() <= 1) {
		result.push_back(s);
		return result;
	}

	//extract tokens
	for (unsigned int i = 0; i < tokInd.size() - 1; i++) {
		result.push_back(s.substr(tokInd[i], tokInd[i + 1] - tokInd[i]));
	}
	result.push_back(s.substr(tokInd[tokInd.size() - 1], s.length() - tokInd[tokInd.size() - 1]));

	return result;
}

vector<vector<string>> TokenUtil::removeBiDirection(vector<string>& tokens) {
	vector<vector<string>> subProps;

	//split proposition into sub propositions on either side of <=>
	vector<string> subProp;
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "<=>") {
			subProps.push_back(subProp);
			subProp.clear();
			continue;
		}

		subProp.push_back(tokens[i]);
	}
	subProps.push_back(subProp);

	//no <=> found
	if (subProps.size() == 1)
		return subProps;

	//implication our two sub props together
	int propASize = subProps[0].size();
	int propBSize = subProps[1].size();

	subProps[0].push_back("=>");
	for (int i = 0; i < propBSize; i++) {
		subProps[0].push_back(subProps[1][i]);
	}

	subProps[1].push_back("=>");
	for (int i = 0; i < propASize; i++) {
		subProps[1].push_back(subProps[0][i]);
	}

	return subProps;
}

bool TokenUtil::compFunc(string a, string b) {
	return a < b;
}

string TokenUtil::getString(vector<string> tokens) {
	string result;

	//sort by alphabetical
	//std::sort(tokens.begin(), tokens.end(), compFunc);

	for (int i = 0; i < tokens.size(); i++) {
		result += tokens[i];
	}

	return result;
}

bool TokenUtil::hasBiDirectional(vector<string>& tokens) {
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "<=>")
			return true;
	}
	return false;
}

int TokenUtil::opPriority(const std::string& token) {
	if (token == "(")
		return -1;

	for (int i = 0; i < 5; i++) {
		if (operators[i] == token)
			return i;
	}

	cerr << "Operator not found" << endl;
	return -1;
}

vector<string> TokenUtil::removeRedundantBrackets(vector<string>& tokens) {
	for (int i = tokens.size() - 1; i >= 0; i--) {
		if (tokens[i] == ")") {
			if (i-3 >= 0
				&& tokens[i-2] == "~"
				&& tokens[i-3] == "(")
			{
				tokens.erase(tokens.begin() + i);
				tokens.erase(tokens.begin() + (i - 3));
				i--;
			}
			else if (i-2 >= 0
				&& tokens[i-2] == "(")
			{
				tokens.erase(tokens.begin() + i);
				tokens.erase(tokens.begin() + (i - 2));
				i--;
			}
		}
	}

	return tokens;
}

string TokenUtil::trimspaces(string& s) {
	for (int i = s.length() - 1; i >= 0; i--) {
		if (s[i] == ' ') {
			s.erase(i, 1);
		}
	}

	return s;
}

string TokenUtil::inverse(const string& q) {
	if (q == "")
		return q;

	if (q[0] == '~')
		return q.substr(1);
	else
		return ("~" + q);
}

string TokenUtil::cnfTokensToString(const vector<string>& tokens) {
	string result = "";

	vector<string> temp;
	bool negated = false;
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "~")
			negated = true;
		else if (isalnum(tokens[i][0]) && negated) {
			temp.push_back("~" + tokens[i]);
			negated = false;
		}
		else if (isalnum(tokens[i][0])) {
			temp.push_back(tokens[i]);
		}
	}

	//sort by alphabetical
	std::sort(temp.begin(), temp.end(), compFunc);

	for (int i = 0; i < temp.size(); i++) {
		result += temp[i];
		if (i + 1 != temp.size()) {
			result += "||";
		}
	}
		

	return result;
}

void TokenUtil::removeDuplicateSymbols(Prop& p) {
	vector<string> tokens = TokenUtil::rpnToInfix(p.getTokens());

	vector<string> signedTokens;
	bool negated = false;
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "~")
			negated = true;
		else if (isalnum(tokens[i][0]) && negated) {
			signedTokens.push_back("~" + tokens[i]);
			negated = false;
		}
		else if (isalnum(tokens[i][0])) {
			signedTokens.push_back(tokens[i]);
		}
	}

	map<string, bool> seen;
	for (int i = signedTokens.size() - 1; i >= 0; i--) {
		if (seen.find(signedTokens[i]) != seen.end()) {
			signedTokens.erase(signedTokens.begin() + i);
		}
		else seen[signedTokens[i]];
	}

	tokens.clear();
	for (int i = 0; i<signedTokens.size(); i++) {
		vector<string> toks = TokenUtil::extractTokens(signedTokens[i]);

		for (string& s : toks) {
			tokens.push_back(s);
		}

		if (i + 1 != signedTokens.size())
			tokens.push_back("||");
	}

	p.setTokens(TokenUtil::infixToRpn(tokens));
}

string TokenUtil::tokensToString(const vector<string>& tokens) {
	string result = "";

	for (int i = 0; i < tokens.size(); i++)
		result += tokens[i];

	return result;
}

string TokenUtil::trimNegation(string signedSymbol) {
	if (signedSymbol[0] == '~')
		return signedSymbol.substr(1);
	else return signedSymbol;
}

bool TokenUtil::isSymbol(const string& s) {
	if (isalnum(s[0]) || (s[0] == '~' && s.size() > 1))
		return true;
	else return false;
}

string TokenUtil::operators[5] = { "<=>", "=>", "||", "&", "~" };
