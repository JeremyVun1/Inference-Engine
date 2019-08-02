#pragma once

#include "KB\KnowledgeBase.h"
#include <string>
#include <fstream>

using namespace std;

class FileParser
{
private:
	static string trim(string& s, const string& trims);
public:
	static void parse(ifstream& inFile, KnowledgeBase& kb, string& q);
};

