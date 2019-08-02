#include "KBFac.h"

KnowledgeBase* KBFac::create(const char* method) {
	if (strcmp(method, "DPLL") == 0 || strcmp(method, "RES") == 0)
		return new CNFKnowledgeBase();
	else
		return new KnowledgeBase();
}

