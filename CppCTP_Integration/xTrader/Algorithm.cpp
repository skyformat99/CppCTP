#include "Algorithm.h"

Algorithm::Algorithm(string is_active) {
	this->is_active = is_active;
}


void Algorithm::setAlgName(string name) {
	this->name = name;
}

string Algorithm::getAlgName() {
	return this->name;
}

void Algorithm::setIsActive(string is_active) {
	this->is_active = is_active;
}

string Algorithm::getIsActive() {
	return this->is_active;
}