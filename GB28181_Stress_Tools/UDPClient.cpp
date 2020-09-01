#include "UDPClient.h"

int UDPClient::bind(const char * ip,int port) {
	return bind(ip,port,"",0);
}

int UDPClient::bind(const char * ip, int port, const char * connect_ip, int connect_port) {

	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		create_client_status = -1;
		return create_client_status;
	}
	client = socket(AF_INET, is_tcp ? SOCK_STREAM:SOCK_DGRAM, is_tcp ? IPPROTO_TCP : IPPROTO_UDP);

	sockaddr_in bind_address;
	bind_address.sin_family = AF_INET;
	bind_address.sin_port = htons(port);
	bind_address.sin_addr.s_addr = inet_addr(ip);
	create_client_status = ::bind(client, (sockaddr*)&bind_address, sizeof(bind_address));
	if (0 != create_client_status) {
		WSACleanup();
	}
	if(!is_tcp){
		return create_client_status;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(connect_port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(connect_ip);
	if (connect(client, (sockaddr *)&serAddr, sizeof(serAddr)) < 0)
	{
		printf("connect error !");
		closesocket(client);
		WSACleanup();
		return -1;
	}
	return 0;
}

void UDPClient::release() {
	if (0 == create_client_status ) {
		closesocket(client);
		WSACleanup();
	}
	create_client_status = -1;
}

void UDPClient::send_packet(const char * target_ip, int target_port, const char * data, int data_length) {
	if(!is_tcp){
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(target_port);
		sin.sin_addr.S_un.S_addr = inet_addr(target_ip);
		sendto(client, data, data_length, 0, (sockaddr *)&sin, sizeof(sin));
	}
	else {
		send(client, data, data_length, 0);
	}
}
