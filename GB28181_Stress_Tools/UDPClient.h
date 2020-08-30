#pragma once
#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

class UDPClient {
	int create_client_status = -1;
	SOCKET client;
public:

	int bind(const char * ip,int port);

	void send_packet(const char * target_ip, int target_port, const char * data, int data_length);

	void release();
};