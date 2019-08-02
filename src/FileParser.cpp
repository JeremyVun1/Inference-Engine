#include "FileParser.h"
#include <sstream>

using namespace std;

string FileParser::trim(string& s, const string& trims) {
	if (s.length() == 0)
		return s;

	size_t start = s.find_first_not_of(trims);
	size_t end = s.find_last_not_of(trims);

	return s.substr(start, end+1);
}

void FileParser::parse(ifstream& inFile, KnowledgeBase& kb, string& q) {
	string buffer, s;
	string trims("\n ;");

	while (getline(inFile, buffer)) {
		if (buffer == "TELL") {
			getline(inFile, buffer);
			istringstream ss(buffer);

			while (getline(ss, s, ';')) {
				s = trim(s, trims);
				kb.tell(s);
			}
		}
		else if (buffer == "ASK") {
			getline(inFile, q);
			q = trim(q, trims);
		}
	}
}
