#pragma once

#include <iostream>
#include <map>
#include "..\KB\KnowledgeBase.h"

using namespace std;

class IEngine
{
public:
	virtual bool ask(KnowledgeBase& kb, const string& q) = 0;
};

