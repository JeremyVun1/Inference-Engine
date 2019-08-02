#include "IEngineFac.h"
#include <iostream>
#include <cctype>
#include <string>

#include "TTEngine.h"
#include "FCEngine.h"
#include "BCEngine.h"
#include "DPLLEngine.h"
#include "ResolutionEngine.h"

using namespace std;

IEngine* IEngineFac::create(const char* method) {
	if (strcmp(method, "TT") == 0)
		return new TTEngine();
	else if (strcmp(method, "FC") == 0)
		return new FCEngine();
	else if (strcmp(method, "BC") == 0)
		return new BCEngine();
	else if (strcmp(method, "DPLL") == 0)
		return new DPLLEngine();
	else if (strcmp(method, "RES") == 0)
		return new ResolutionEngine();
	else {
		cout << "invalid method: choose from TT, FC, BC, DPLL, RES" << endl;
		throw "invalid method: choose from TT, FC, BC, DPLL, RES";
	}
}
