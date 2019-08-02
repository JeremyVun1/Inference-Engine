#include "PropCount.h"

/*
LOCALS
	Prop prop;
	int count;
*/

PropCount::PropCount(Prop prop, int count) : prop(prop), count(count) {}

bool PropCount::symbolInPremises(const string& s) const {
	for (string& propSym : prop.getSignedPremises()) {
		if (s == propSym)
			return true;
	}
	return false;
}

bool PropCount::evalPremises() {
	return prop.evalPremises();
}

string PropCount::getHead() const {
	return prop.getHead();
}

string PropCount::getSignedHead() const {
	return prop.getSignedHead();
}