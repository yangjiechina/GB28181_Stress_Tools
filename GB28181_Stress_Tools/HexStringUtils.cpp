#include "HexStringUtils.h"

std::string binToHex(unsigned char *data, size_t size) {
	std::ostringstream strHex;
	strHex << std::hex << std::setfill('0');
	for (size_t i = 0; i < size; ++i) {
		strHex << std::setw(2) << static_cast<unsigned int>(data[i]);
	}
	return strHex.str();
}