#include "UDPClient.h"

int UDPClient::bind(const char * ip,int port) {
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		create_client_status = -1;
		return create_client_status;
	}
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in bind_address;
	bind_address.sin_family = AF_INET;
	bind_address.sin_port = htons(port);
	bind_address.sin_addr.s_addr = inet_addr(ip);
	create_client_status = ::bind(client, (sockaddr*)&bind_address, sizeof(bind_address));
	if (0 != create_client_status) {
		WSACleanup();
	}
	return create_client_status;
}

void UDPClient::release() {
	if (0 == create_client_status ) {
		closesocket(client);
		WSACleanup();
	}
	create_client_status = -1;
}

void UDPClient::send_packet(const char * target_ip, int target_port, const char * data, int data_length) {
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(target_port);
	sin.sin_addr.S_un.S_addr = inet_addr(target_ip);
	sendto(client, data, data_length, 0, (sockaddr *)&sin, sizeof(sin));
}