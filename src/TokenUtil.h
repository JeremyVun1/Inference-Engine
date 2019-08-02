#pragma once

#include <vector>
#include <string>
#include <stack>
#include "KB\Prop.h"

using namespace std;

class TokenUtil
{
private:
	static string operators[5];

	//rpn to inifix helper
	static string operate(string& op, stack<string>& s, stack<int>& ops);
public:	
	//conversions
	static vector<string> infixToRpn(const vector<string>& tokens);
	static vector<string> rpnToInfix(const vector<string>& tokens);

	static void removeDuplicateSymbols(Prop& p);

	//alphabet sorter
	static string cnfTokensToString(const vector<string>& tokens);
	static bool compFunc(string a, string b);

	//create tokens from a sentence string
	static vector<string> extractTokens(const string& s);
	static vector<vector<string>> removeBiDirection(vector<string>& tokens);
	
	static string getString(vector<string> tokens);

	//determine priority of an operator
	static int opPriority(const std::string& opTok);

	static string trimspaces(string& s);

	static string inverse(const string& q);

	static string tokensToString(const vector<string>& tokens);

	static string trimNegation(string signedSymbol);

	static bool isSymbol(const string& s);

	static vector<string> removeRedundantBrackets(vector<string>& tokens);

	static bool hasBiDirectional(vector<string>& tokens);
};

