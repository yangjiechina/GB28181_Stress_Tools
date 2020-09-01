#pragma once
#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

class UDPClient {
	int create_client_status = -1;
	SOCKET client;
	bool is_tcp;
	
public:

	UDPClient(bool is_tcp) {
		this->is_tcp = is_tcp;
	}

	int bind(const char * ip,int port);

	int bind(const char * ip, int port, const char * connect_ip,int connect_port);

	void send_packet(const char * target_ip, int target_port, const char * data, int data_length);

	void release();
};
