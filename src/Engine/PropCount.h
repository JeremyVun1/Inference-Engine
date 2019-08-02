#pragma once

#include "..\KB\Prop.h"

struct PropCount
{
	Prop prop;
	int count;

	PropCount(Prop prop, int count);

	bool symbolInPremises(const string& s) const;
	bool evalPremises();
	string getHead() const;
	string getSignedHead() const;
};

