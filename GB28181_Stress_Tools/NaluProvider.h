#pragma once
#include "Nalu.h"
#include <vector>
#include <Windows.h>

class NaluProvider {

private :

	std::vector<Nalu*>* nalu_vector;


	CRITICAL_SECTION lock;

public:
	NaluProvider(std::vector<Nalu*>* nalu_vector);
	bool get_nalu(int index, Nalu* target);
}
;

