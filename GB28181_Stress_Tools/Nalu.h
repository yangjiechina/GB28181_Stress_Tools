#pragma once
#include "NaluType.h"
class Nalu {
public:
	char * packet;
	int length;
	NaluType type;

	~Nalu() {
		if (packet != nullptr) {
			delete packet;
			packet = nullptr;
		}
	}
};