#pragma once

#include <string>
#include "KnowledgeBase.h"
#include "CNFKnowledgeBase.h"

class KBFac
{
public:
	static KnowledgeBase* create(const char* method);
};

