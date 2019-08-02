#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <sstream>
#include <chrono>

#include "Engine\IEngine.h"
#include "Engine\IEngineFac.h"
#include "KB\KnowledgeBase.h"
#include "KB\KBFac.h"

#include "FileParser.h"

#include "KB\IProp.h"

using namespace std;

using namespace std::chrono;


int main(int argc, char* argv[]) {
	//guards
	if (argc < 3) {
		cerr << "Invalid args: <exe> <method> <filename>" << endl;
		return 1;
	}

	cout << "Opening file: " << argv[2] << endl;

	ifstream inFile(argv[2]);
	if (!inFile.good()) {
		cerr << "Cannot open file" << endl;
		inFile.close();
		return 3;
	}

	//create specified engine and kb
	IEngine* engine = IEngineFac::create(argv[1]);
	KnowledgeBase* kb = KBFac::create(argv[1]);

	//parse text file into kb and query
	string q;
	FileParser::parse(inFile, *kb, q);
	inFile.close();

	//query
	auto start = std::chrono::high_resolution_clock::now();
	engine->ask(*kb, q);
	auto end = std::chrono::high_resolution_clock::now();

	//record elapsed time
	chrono::duration<int64_t, nano> elapsed = end - start;
	cout << endl << "time (ms): " << (elapsed.count() / 1000000.0) << endl;

	delete engine;
	delete kb;

	return 0;
}