#pragma once

#include "IEngine.h"

class IEngineFac
{
public:
	static IEngine* create(const char* method);
};

