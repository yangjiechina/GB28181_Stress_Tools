#pragma once
#include "Device.h"
#include "LoadH264.h"
#include "NaluProvider.h"
#include <iostream>
#include <string>
#include <thread>
using namespace std;

int main() {
	//const char * h264_path = "tmp.264";
	const char * h264_path = "bigbuckbunnynoB_480x272.h264";
	int status = load(h264_path);
	//Device * device = new Device("34020000001320000001","34020000002000000001","49.235.63.67",5066,
	//	"123456");

	const char * ip = "192.168.110.150";
	//const char * ip = "192.168.0.107";
	//const char * ip = "49.235.63.67";
	int port = 15060;
	const char * password = "12345678";
	/*Device * device = new Device("34020000001320000001", "34020000002000000001", ip, 5066,"123456");
	device->start_sip_client(50000);*/

	//不采用一个Device 一个h264队列的方式
	//采用同步访问h264帧函数，为了不影响效率，按照比例(20个Device:1个h264)创建队列；

	//多少Deivce对应一个队列
	int rate = 20;
	int device_count = 40;
	
	int start_port = 50000;

	extern vector<Nalu*> nalu_vector;
	    
	vector<NaluProvider*> nalu_vector_vector((device_count -1) / rate +1 );
	

	for (int i = 0; i < device_count; i++) {
		
		//拷贝h264队列
		if (i / rate == 0) {
			int size = nalu_vector.size();
			vector<Nalu*>* current_nalu_vector = new vector<Nalu*>();
				for (int i = 0; i < size; i++) {
					Nalu * nalu = nalu_vector.at(i);

					char * copy_packet = (char *)malloc(nalu->length * sizeof(char));

					memcpy(copy_packet, nalu->packet, nalu->length);

					Nalu * copy_nalu = new Nalu();
					copy_nalu->packet = copy_packet;
					copy_nalu->length = nalu->length;
					copy_nalu->type = nalu->type;
					current_nalu_vector->push_back(copy_nalu);
				}
				NaluProvider* provider = new NaluProvider(current_nalu_vector);
				nalu_vector_vector.push_back(provider);
		}
		string prefix = "3402000000132000000";
		Device * device = new Device(prefix.append(to_string(i)).c_str(), "34020000002000000001", ip, port,
			password, nalu_vector_vector.back());
		device->start_sip_client(++start_port);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	getchar();
	return 0;
}