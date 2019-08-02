#include "CNFKnowledgeBase.h"
#include <iostream>

CNFKnowledgeBase::CNFKnowledgeBase() { }

void CNFKnowledgeBase::tell(string sentence)
{
	if (sentence == "") {
		return;
	}

	//convert to cnf
	vector<vector<string>> clauses = convertToCNF(sentence);

	//add clauses to propositions
	for (int i = 0; i < clauses.size(); i++) {
		clauses[i] = TokenUtil::removeRedundantBrackets(clauses[i]);
		//register our symbols and convert to rpn
		registerSymbols(clauses[i]);

		//create proposition
		Prop p(TokenUtil::infixToRpn(clauses[i]), propositions, getClauseSentence(clauses[i]));
		propositions.insert(pair<string, Prop>(p.getSignedId(), p));
	}
}

string CNFKnowledgeBase::getClauseSentence(vector<string>& tokens) {
	string result = "";

	for (string& s : tokens)
		result += s;

	return result;
}

vector<vector<string>> CNFKnowledgeBase::convertToCNF(string& sentence) {
	//split sentence into tokens
	sentence = TokenUtil::trimspaces(sentence);
	vector<string> tokens = TokenUtil::extractTokens(sentence);

	//1. eliminiate <=>
	tokens = removeBiDirection(tokens);

	vector<vector<string>> clauses{ tokens };

	for (int i = clauses.size() - 1; i >= 0; i--) {
		//2. eliminate =>
		clauses[i] = removeImplication(clauses[i]);
		clauses[i] = TokenUtil::removeRedundantBrackets(clauses[i]);

		//3. push negations in
		clauses[i] = pushNegations(clauses[i]);
		
		//remove unnecessary brackets by converting between infix and postfix
		clauses[i] = TokenUtil::infixToRpn(clauses[i]);
		clauses[i] = TokenUtil::rpnToInfix(clauses[i]);

		//4. apply distributive law
		vector<vector<string>> distributedClauses = distributive(clauses[i]);

		//replace our clause with new distributed clauses if it can be distributed
		if (distributedClauses.size() > 0) {
			clauses.erase(clauses.begin() + i);
			for (int j = 0; j < distributedClauses.size(); j++) {
				clauses.push_back(distributedClauses[j]);
			}
		}
	}

	return clauses;
}

vector<vector<string>> CNFKnowledgeBase::distributive(vector<string>& tokens) {
	vector<vector<string>> result;

	while (hasAnd(tokens)) {
		//p || l & r = p||l & p||r
		vector<string> r;
		vector<string> l;
		vector<string> p;

		int lStart = 0;
		int rEnd = 0;

		//get index of and operator
		int andIndex = 0;
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens[i] == "&") {
				andIndex = i;
				break;
			}
		}

		//get r
		int nb = 0;
		if (tokens[andIndex + 1] == "(") {
			//get the contents of the brackets
			for (int i = andIndex + 2; i < tokens.size(); i++) {
				if (tokens[i] == "(") {
					r.push_back(tokens[i]);
					nb++;
				}
				else if (nb > 0 && tokens[i] == ")") {
					r.push_back(tokens[i]);
					nb--;
				}
				else if (nb == 0 && tokens[i] == ")") {
					rEnd = i;
					break;
				}
				else {
					r.push_back(tokens[i]);
				}
			}
		}
		else {
			//get next unbracketed expression
			if (tokens[andIndex + 1] == "~") {
				r.push_back(tokens[andIndex + 1]);
				r.push_back(tokens[andIndex + 2]);
				rEnd = andIndex + 3;
			}
			else {
				r.push_back(tokens[andIndex + 1]);
				rEnd = andIndex + 2;
			}
		}

		if (hasAnd(r)) {
			//recurse and see if our clause can be cnfed more
			vector<vector<string>> rightClauses = distributive(r);

			//replace r with right clauses;
			vector<string> temp;
			for (int i = 0; i < andIndex + 1; i++) {
				temp.push_back(tokens[i]);
			}

			//add new sub cnf clauses
			temp.push_back("(");
			for (int i = 0; i < rightClauses.size(); i++) {
				for (int j = 0; j < rightClauses[i].size(); j++) {
					temp.push_back(rightClauses[i][j]);
				}

				if (i + 1 < rightClauses.size()) {
					temp.push_back(")");
					temp.push_back("&");
					temp.push_back("(");
				}
			}
			temp.push_back(")");

			for (int i = rEnd; i < tokens.size(); i++) {
				temp.push_back(tokens[i]);
			}
			tokens = temp;
			continue;
		}
		

		//get l
		nb = 0;
		if (tokens[andIndex - 1] == ")") {
			//get previous bracketed expression
			for (int i = andIndex - 2; i >= 0; i--) {
				if (tokens[i] == ")") {
					l.push_back(tokens[i]);
					nb++;
				}
				else if (nb > 0 && tokens[i] == "(") {
					l.push_back(tokens[i]);
					nb--;
				}
				else if (tokens[i] == "(") {
					lStart = i;
					break;
				}
				else {
					l.push_back(tokens[i]);
				}
			}

			vector<string> temp = l; //reverse left side
			l.clear();
			for (int i = temp.size()-1; i >= 0; i--) {
				l.push_back(temp[i]);
			}
		}
		else {
			//get previous unbracketed expression
			if (andIndex-2 >= 0 && tokens[andIndex - 2] == "~") {
				l.push_back(tokens[andIndex - 2]);
				l.push_back(tokens[andIndex - 1]);
				lStart = andIndex - 2;
			}
			else {
				l.push_back(tokens[andIndex - 1]);
				lStart = andIndex - 1;
			}
		}

		if (hasAnd(l)) {
			//recurse and see if our clause can be cnfed more
			vector<vector<string>> leftClauses = distributive(r);

			//replace l with new cnf clauses;
			vector<string> temp;
			//0..lStart..andIndex
			if (lStart > 0) {
				for (int i = 0; i < lStart + 1; i++) {
					temp.push_back(tokens[i]);
				}
			}

			temp.push_back("(");
			for (int i = 0; i < leftClauses.size(); i++) {
				for (int j = 0; j < leftClauses[i].size(); j++) {
					temp.push_back(leftClauses[i][j]);
				}

				if (i + 1 < leftClauses.size()) {
					temp.push_back(")");
					temp.push_back("&");
					temp.push_back("(");
				}
			}
			temp.push_back(")");

			for (int i = andIndex; i < tokens.size(); i++) {
				temp.push_back(tokens[i]);
			}
			tokens = temp;
			continue;
		}

		//get p (could be right or left of the and)
		bool orFound = false;
		bool orIsLeft = false;
		int orIndex = 0;

		//search for OR to the left
		if (lStart > 0) {
			for (int i = lStart; i >= 0; i--) {
				if (tokens[i] == "||") {
					orIndex = i;
					orIsLeft = true;
					orFound = true;
					break;
				}
				else if (tokens[i] == "&") {
					break;
				}
			}
		}

		//search for OR to the right
		if (!orFound && rEnd < tokens.size()) {
			for (int i = rEnd; i < tokens.size(); i++) {
				if (tokens[i] == "||") {
					orIndex = i;
					orIsLeft = false;
					orFound = true;
					break;
				}
				else if (tokens[i] == "&") {
					break;
				}
			}
		}

		//no or found, cannot distribute
		if (!orFound) {
			l = TokenUtil::removeRedundantBrackets(l);
			r = TokenUtil::removeRedundantBrackets(r);
			result.push_back(l);
			result.push_back(r);

			//remove clauses from token openset
			vector<string> temp;
			if (lStart > 0) {
				for (int i = 0; i < lStart; i++) {
					temp.push_back(tokens[i]);
				}
			}

			if (rEnd + 3 < tokens.size() && tokens[rEnd + 1] == ")" && !isalnum(tokens[rEnd + 2][0])) {
				temp.push_back(tokens[rEnd + 1]);
				rEnd += 1;
			}
			else if (rEnd + 2 < tokens.size() && tokens[rEnd] == ")" && tokens[rEnd + 1] == "&") {
				temp.push_back(")");
			}

			for (int i = rEnd + 2; i < tokens.size(); i++) {
				temp.push_back(tokens[i]);
			}

			//if no more clauses to examine, then return all
			if (temp.empty())
				return result;
			else {
				//prune unnecessary brackets
				temp = TokenUtil::infixToRpn(temp);
				tokens = TokenUtil::rpnToInfix(temp);

				continue;
			}
		}

		//get the p
		int orStart = 0;
		int orEnd = 0;
		if (orIsLeft) {
			//get expression to the left of the Or index
			if (tokens[orIndex - 1] == ")") {
				//get previous bracketed expression
				nb = 0;
				for (int i = orIndex - 2; i >= 0; i--) {
					if (tokens[i] == ")") {
						p.push_back(tokens[i]);
						nb++;
					}
					else if (nb > 0 && tokens[i] == "(") {
						p.push_back(tokens[i]);
						nb--;
					}
					else if (nb == 0 && tokens[i] == "(") {
						orStart = i;
						break;
					}
					else {
						p.push_back(tokens[i]);
					}
				}

				vector<string> temp = p; //reverse
				p.clear();
				for (int i = temp.size() - 1; i >= 0; i--) {
					p.push_back(temp[i]);
				}
			}
			else {
				//get previous unbracketed expression

				nb = 0;
				for (int i = orIndex - 1; i >= 0; i--) {
					if (tokens[i] == ")") {
						nb++;
						p.push_back(tokens[i]);
					}
					else if (tokens[i] == "(") {
						nb--;
						if (nb < 0) {
							orStart = i + 1;
							break;
						}
						else p.push_back(tokens[i]);
					}
					else p.push_back(tokens[i]);

					if (i == 0)
						orStart = 0;
				}

				vector<string> temp = p; //reverse left side
				p.clear();
				for (int i = temp.size() - 1; i >= 0; i--) {
					p.push_back(temp[i]);
				}

				/*
				if (orIndex-2 >= 0 && tokens[orIndex - 1] == "~") {
					p.push_back(tokens[orIndex - 2]);
					p.push_back(tokens[orIndex - 1]);
					orStart = orIndex - 2;
				}
				else {
					p.push_back(tokens[orIndex - 1]);
					orStart = orIndex - 1;
				}
				*/
			}
		}
		else {
			//get expression to the right of the Or index
			if (tokens[orIndex + 1] == "(") {
				//get the contents of the brackets
				nb = 0;
				for (int i = orIndex + 2; i < tokens.size(); i++) {
					if (tokens[i] == "(") {
						p.push_back(tokens[i]);
						nb++;
					}
					else if (nb > 0 && tokens[i] == ")") {
						p.push_back(tokens[i]);
						nb--;
					}
					else if (nb == 0 && tokens[i] == ")") {
						orEnd = i;
						break;
					}
					else {
						p.push_back(tokens[i]);
					}
				}
			}
			else {
				//get next unbracketed expression

				nb = 0;
				for (int i = orIndex + 1; i < tokens.size(); i++) {
					if (tokens[i] == "(") {
						nb++;
						p.push_back(tokens[i]);
					}
					else if (tokens[i] == ")") {
						nb--;
						if (nb < 0) {
							orEnd = i - 1;
							break;
						}
						else p.push_back(tokens[i]);
					}
					else p.push_back(tokens[i]);

					if (i == tokens.size() - 1)
						orEnd = i;
				}

				/*
				if (tokens[orIndex + 1] == "~") {
					p.push_back(tokens[orIndex + 1]);
					p.push_back(tokens[orIndex + 2]);
					orEnd = orIndex + 2;
				}
				else {
					p.push_back(tokens[orIndex + 1]);
					orEnd = orIndex + 1;
				}

				*/
			}
		}

		//distribute p, r, l
		r = distribute(p, r);
		l = distribute(p, l);

		vector<string> newTokens;
		if (orIsLeft) {
			//the unchanged beginning of the tokens
			if (orStart > 0) {
				for (int i = 0; i < orStart; i++) {
					newTokens.push_back(tokens[i]);
				}
			}

			//distributed left and right
			for (int i = 0; i < l.size(); i++) {
				newTokens.push_back(l[i]);
			}
			newTokens.push_back("&");
			for (int i = 0; i < r.size(); i++) {
				newTokens.push_back(r[i]);
			}

			//the rest of the tokens
			if (rEnd + 1 < tokens.size()) {
				for (int i = rEnd + 1; i < tokens.size(); i++) {
					newTokens.push_back(tokens[i]);
				}
			}
		}
		else {
			//beginning of token string
			if (lStart > 0) {
				for (int i = 0; i < lStart; i++) {
					newTokens.push_back(tokens[i]);
				}
			}

			//distributed left and right
			for (int i = 0; i < l.size(); i++) {
				newTokens.push_back(l[i]);
			}
			newTokens.push_back("&");
			for (int i = 0; i < r.size(); i++) {
				newTokens.push_back(r[i]);
			}

			//the rest of the tokens
			if (orEnd + 1 < tokens.size()) {
				for (int i = orEnd + 1; i < tokens.size(); i++) {
					newTokens.push_back(tokens[i]);
				}
			}
		}

		tokens = newTokens;
	}

	result.push_back(tokens);

	return result;
}

vector<string> CNFKnowledgeBase::distribute(vector<string> orTokens, vector<string> andTokens) {
	//p || l & r = p||l & p||r
	vector<string> result;

	result.push_back("(");

	for (int i = 0; i < orTokens.size(); i++) {
		result.push_back(orTokens[i]);
	}
	result.push_back("||");
	for (int i = 0; i < andTokens.size(); i++) {
		result.push_back(andTokens[i]);
	}

	result.push_back(")");

	return result;
}

//helper method for distributive law
//check if there is an & in our clause. If no &, then no distributive needs to be done
bool CNFKnowledgeBase::hasAnd(vector<string> clause) {
	for (int i = 0; i < clause.size(); i++) {
		if (clause[i] == "&")
			return true;
	}

	return false;
}

vector<string> CNFKnowledgeBase::pushNegations(vector<string> tokens) {

	bool negationFound = false;
	int nb = 0;
	int start = 0;
	int finish = 0;
	vector<string> clause;

	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "~" && tokens[i + 1] == "(") {
			negationFound = true;
			start = i;
			nb++;
			i++;
		}
		else if (tokens[i] == ")" && negationFound) {
			nb--;
			finish = i;
		}
		else if (nb == 1) {
			clause.push_back(tokens[i]);
		}
	}

	if (clause.empty())
		return tokens;

	clause = invertClause(clause);

	vector<string> result;

	//rebuild tokens
	if (start > 0) {
		for (int i = 0; i < start; i++) {
			result.push_back(tokens[i]);
		}

		for (int i = 0; i < clause.size(); i++) {
			result.push_back(clause[i]);
		}

		if (finish + 1 < tokens.size()) {
			for (int i = finish + 1; i < tokens.size(); i++) {
				result.push_back(tokens[i]);
			}
		}
	}
	else {
		result = clause;

		if (finish + 1 < tokens.size()) {
			for (int i = finish + 1; i < tokens.size(); i++) {
				result.push_back(tokens[i]);
			}
		}
	}

	return result;
}

//deprecated attempt at CNF
vector<vector<string>> CNFKnowledgeBase::parseClauses(vector<string>& baseTokens, int begin, int end) {

	vector<string> tokens;
	for (int i = begin; i < end; i++) {
		tokens.push_back(baseTokens[i]);
	}

	//remove extra brackets
	tokens = TokenUtil::infixToRpn(tokens);
	tokens = TokenUtil::rpnToInfix(tokens);

	vector<vector<string>> result;

	int nb = 0;
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "(")
			nb++;
		else if (tokens[i] == ")")
			nb--;

		//split expression on exposed & (no parenthesis around it)
		else if (nb == 0 && tokens[i] == "&") {
			vector<vector<string>> leftClauses = parseClauses(tokens, 0, i);
			vector<vector<string>> rightClauses = parseClauses(tokens, i + 1, tokens.size());

			for (int j = 0; j < leftClauses.size(); j++) {
				result.push_back(leftClauses[j]);
			}

			for (int j = 0; j < rightClauses.size(); j++) {
				result.push_back(rightClauses[j]);
			}
		}
	}

	if (result.empty())
		result.push_back(tokens);

	return result;
}

vector<string> CNFKnowledgeBase::removeBiDirection(vector<string>& tokens) {
	vector<vector<string>> subProps;

	//split proposition into sub propositions on either side of <=>
	vector<string> subProp;

	while (TokenUtil::hasBiDirectional(tokens)) {
		//get index of bidirection
		int bdIndex = -1;
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens[i] == "<=>") {
				bdIndex = i;
				break;
			}
		}

		if (bdIndex == -1) {
			return tokens;
		}

		//get left side
		vector<string> left;
		int nb = 0;
		int lStart = 0;

		//get bracketed contents
		if (tokens[bdIndex - 1] == ")") {
			left.push_back(tokens[bdIndex - 1]);

			for (int i = bdIndex - 2; i >= 0; i--) {
				if (tokens[i] == ")") {
					left.push_back(tokens[i]);
					nb++;
				}
				else if (nb > 0 && tokens[i] == "(") {
					left.push_back(tokens[i]);
					nb--;
				}
				else if (nb == 0 && tokens[i] == "(") {
					left.push_back(tokens[i]);
					lStart = i;
					break;
				}
				else {
					left.push_back(tokens[i]);
				}
			}

			vector<string> temp = left; //reverse left side
			left.clear();
			for (int i = temp.size() - 1; i >= 0; i--) {
				left.push_back(temp[i]);
			}
		}
		else {
			if (bdIndex - 2 >= 0 && tokens[bdIndex - 2] == "~") {
				left.push_back(tokens[bdIndex - 2]);
				left.push_back(tokens[bdIndex - 1]);
				lStart = bdIndex - 2;
			}
			else {
				left.push_back(tokens[bdIndex - 1]);
				lStart = bdIndex - 1;
			}
		}

		//get right side
		vector<string> right;
		int rEnd = 0;
		nb = 0;
		if (tokens[bdIndex + 1] == "(") {
			right.push_back(tokens[bdIndex + 1]);

			//get the contents of the brackets
			for (int i = bdIndex + 2; i < tokens.size(); i++) {
				if (tokens[i] == "(") {
					right.push_back(tokens[i]);
					nb++;
				}
				else if (nb > 0 && tokens[i] == ")") {
					right.push_back(tokens[i]);
					nb--;
				}
				else if (nb == 0 && tokens[i] == ")") {
					right.push_back(tokens[i]);
					rEnd = i;
					break;
				}
				else {
					right.push_back(tokens[i]);
				}
			}
		}
		else {
			//get next unbracketed expression
			if (tokens[bdIndex + 1] == "~") {
				right.push_back(tokens[bdIndex + 1]);
				right.push_back(tokens[bdIndex + 2]);
				rEnd = bdIndex + 2;
			}
			else {
				right.push_back(tokens[bdIndex + 1]);
				rEnd = bdIndex + 1;
			}
		}

		vector<string> PropA = implicate(left, right);
		vector<string> PropB = implicate(right, left);

		//rebuild tokens
		vector<string> newTokens;
		
		//beginning tokens
		if (lStart > 0) {
			for (int i = 0; i < lStart; i++) {
				newTokens.push_back(tokens[i]);
			}
		}

		//left and right implication children
		for (int i = 0; i < PropA.size(); i++) {
			newTokens.push_back(PropA[i]);
		}
		newTokens.push_back("&");
		for (int i = 0; i < PropB.size(); i++) {
			newTokens.push_back(PropB[i]);
		}

		//trailing tokens
		if (rEnd + 1 < tokens.size()) {
			for (int i = rEnd + 1; i < tokens.size(); i++) {
				newTokens.push_back(tokens[i]);
			}
		}

		tokens = newTokens;
	}

	return tokens;
}

vector<string> CNFKnowledgeBase::implicate(vector<string>& left, vector<string>& right) {
	vector<string> result;

	result.push_back("(");
	
	for (int i = 0; i < left.size(); i++) {
		result.push_back(left[i]);
	}
	result.push_back("=>");
	for (int i = 0; i < right.size(); i++) {
		result.push_back(right[i]);
	}

	result.push_back(")");

	return result;
}

vector<string> CNFKnowledgeBase::getPremises(const vector<string>& tokens) {
	vector<string> result;

	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "(") {
			result.clear();
			continue;
		}
		else if (tokens[i] == ")") {
			continue;
		}
		else if (tokens[i] == "=>" || tokens[i] == "<=>")
			break;
		else result.push_back(tokens[i]);
	}

	return result;
}

// input/output is infix
vector<string> CNFKnowledgeBase::invertClause(const vector<string>& tokens) {
	vector<string> result;

	for (int i = 0; i < tokens.size(); i++) {
		if (isalnum(tokens[i][0])) {
			if (i > 0 && tokens[i - 1] == "~")
				result.push_back(tokens[i]);
			else {
				result.push_back("~");
				result.push_back(tokens[i]);
			}
		}
		else if (tokens[i] == "&")
			result.push_back("||");
		else if (tokens[i] == "||")
			result.push_back("&");
	}

	return result;
}

vector<string> CNFKnowledgeBase::getHead(vector<string>& tokens) {
	vector<string> result;

	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "=>" || tokens[i] == "<=>") {
			result.clear();
			continue;
		}
		else {
			result.push_back(tokens[i]);
		}
	}

	return result;
}

bool CNFKnowledgeBase::hasImplication(const vector<string>& tokens) const {
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "=>")
			return true;
	}

	return false;
}

vector<string> CNFKnowledgeBase::removeImplication(vector<string>& tokens) {
	while (hasImplication(tokens)) {

		//get implication index
		int impIndex = -1;
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens[i] == "=>") {
				impIndex = i;
				break;
			}
		}

		//no implication found
		if (impIndex == -1)
			return tokens;

		//get left side of implication
		vector<string> left;
		int nb = 0;
		int lStart = 0;
		if (tokens[impIndex - 1] == ")") {
			//get contents of bracket
			left.push_back(tokens[impIndex - 1]);

			for (int i = impIndex - 2; i >= 0; i--) {
				if (tokens[i] == ")") {
					left.push_back(tokens[i]);
					nb++;
				}
				else if (nb > 0 && tokens[i] == "(") {
					left.push_back(tokens[i]);
					nb--;
				}
				else if (nb == 0 && tokens[i] == "(") {
					left.push_back(tokens[i]);
					lStart = i;
					break;
				}
				else left.push_back(tokens[i]);
			}

			vector<string> temp = left; //reverse left side
			left.clear();
			for (int i = temp.size() - 1; i >= 0; i--) {
				left.push_back(temp[i]);
			}
		}
		else {
			nb = 0;
			for (int i = impIndex - 1; i >= 0; i--) {
				if (tokens[i] == ")") {
					nb++;
					left.push_back(tokens[i]);
				}
				else if (tokens[i] == "(") {
					nb--;
					if (nb < 0) {
						lStart = i+1;
						break;
					}
					else left.push_back(tokens[i]);
				}
				else left.push_back(tokens[i]);

				if (i == 0)
					lStart = 0;
			}

			vector<string> temp = left; //reverse left side
			left.clear();
			for (int i = temp.size() - 1; i >= 0; i--) {
				left.push_back(temp[i]);
			}

			/*
			if (impIndex - 2 >= 0 && tokens[impIndex - 2] == "~") {
				left.push_back(tokens[impIndex - 2]);
					left.push_back(tokens[impIndex - 1]);
					lStart = impIndex - 2;
				}
				else {
					left.push_back(tokens[impIndex - 1]);
					lStart = impIndex - 1;
				}
			*/
		}

		//get right side of implication
		vector<string> right;
		nb = 0;
		int rEnd = 0;
		if (tokens[impIndex + 1] == "(") {
			//get contents of bracket
			right.push_back(tokens[impIndex + 1]);

			for (int i = impIndex + 2; i < tokens.size(); i++) {
				if (tokens[i] == "(") {
					right.push_back(tokens[i]);
					nb++;
				}
				else if (nb > 0 && tokens[i] == ")") {
					right.push_back(tokens[i]);
					nb--;
				}
				else if (nb == 0 && tokens[i] == ")") {
					right.push_back(tokens[i]);
					rEnd = i;
					break;
				}
				else right.push_back(tokens[i]);
			}
		}
		else {
			nb = 0;
			for (int i = impIndex + 1; i < tokens.size(); i++) {
				if (tokens[i] == "(") {
					nb++;
					right.push_back(tokens[i]);
				}
				else if (tokens[i] == ")") {
					nb--;
					if (nb < 0) {
						rEnd = i-1;
						break;
					}
					else right.push_back(tokens[i]);
				}
				else right.push_back(tokens[i]);

				if (i == tokens.size() - 1)
					rEnd = i;
			}

			/*
			if (impIndex + 2 < tokens.size() && tokens[impIndex + 1] == "~") {
				right.push_back(tokens[impIndex + 1]);
				right.push_back(tokens[impIndex + 2]);
				rEnd = impIndex + 2;
			}
			else {
				right.push_back(tokens[impIndex + 1]);
				rEnd = impIndex + 1;
			}
			*/
		}
		
		left = invertClause(left);

		//rebuild tokens
		vector<string> newTokens;
		if (lStart > 0) {
			for (int i = 0; i < lStart; i++) {
				newTokens.push_back(tokens[i]);
			}
		}

		//push left side of implication
		for (int i = 0; i < left.size(); i++) {
			newTokens.push_back(left[i]);
		}
		newTokens.push_back("||");
		newTokens.push_back("(");

		//push right side of implication
		for (int i = 0; i < right.size(); i++) {
			newTokens.push_back(right[i]);
		}
		newTokens.push_back(")");

		//push rest of tokens
		if (rEnd + 1 < tokens.size()) {
			for (int i = rEnd+1; i < tokens.size(); i++) {
				newTokens.push_back(tokens[i]);
			}
		}

		tokens = TokenUtil::removeRedundantBrackets(newTokens);

		/*
		for (int i = 0; i < tokens.size(); i++) {
			if (tokens[i] == "&")
				std::cout << "/\\";
			else std::cout << tokens[i];
		}
		std::cout << std::endl;
		*/
	}

	return tokens;
}
