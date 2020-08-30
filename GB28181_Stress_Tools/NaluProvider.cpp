#include "NaluProvider.h"


NaluProvider::NaluProvider(std::vector<Nalu*>* nalu_vector) :nalu_vector(nalu_vector){
	InitializeCriticalSection(&lock); 

};

bool NaluProvider::get_nalu(int index, Nalu* target) {

	EnterCriticalSection(&lock);
	if (nalu_vector != nullptr && index >= nalu_vector->size()) {
		return false;
	}
	Nalu * nalu = nalu_vector->at(index);

	memcpy(target->packet,nalu->packet,nalu->length);

	target->length = nalu->length;
	target->type = nalu->type;
	LeaveCriticalSection(&lock);
	return true;

}