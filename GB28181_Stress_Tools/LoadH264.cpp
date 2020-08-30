#include "LoadH264.h"
extern "C" {
#include "h264_parser.h"
}

std::vector<Nalu*> nalu_vector;

char sps_data[128] = { 0 };
int sps_data_length = 0;

char pps_data[128] = { 0 };
int pps_data_length = 0;

void out_nalu(char * buffer, int size, NaluType naluType) {

	//std::cout << "rtp_packet >>> " << binToHex((unsigned char *)buffer, size);

	if (NALU_TYPE_SPS == naluType) {
		memcpy(sps_data, buffer, size);
		sps_data_length = size;
		return;
	}

	if (NALU_TYPE_PPS == naluType) {
		memcpy(pps_data, buffer, size);
		pps_data_length = size;
		return;
	}
	Nalu * nalu = new Nalu;

	bool is_i_frame = (NALU_TYPE_IDR == naluType);

	char * packet = (char *)malloc(is_i_frame ? (size + sps_data_length + pps_data_length) : size * sizeof(char));
	if (is_i_frame) {

		memcpy(packet, sps_data, sps_data_length);

		memcpy(packet + sps_data_length, pps_data, pps_data_length);

		memcpy(packet + sps_data_length + pps_data_length, buffer, size);

		size += (sps_data_length + pps_data_length);
	}
	else {
		memcpy(packet, buffer, size);
	}
	nalu->packet = packet;
	nalu->length = size;
	nalu->type = naluType;

	nalu_vector.push_back(nalu);
}

int load(const char * path) {
	return simplest_h264_parser(path, out_nalu);
}