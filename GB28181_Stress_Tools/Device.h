#pragma once
#define WIN32_INTEROP_TYPES_H
extern "C" {
	//#include <Win32_Interop/win32fixes.h>
#include <eXosip2/eXosip.h>
}
#include <winsock2.h>
#include "UDPClient.h"
#include "NaluProvider.h"
#include <string>
#include <vector>
#include <functional>
#include "Message.h"
#include <thread>
#include <memory>
#include <condition_variable>
#include <mutex>

//----exosip----//

#pragma comment(lib, "eXosip.lib")
#pragma comment(lib, "libcares.lib")
#pragma comment(lib, "osip2.lib")
#pragma comment(lib, "osipparser2.lib")

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Qwave.lib")
#pragma comment(lib, "delayimp.lib")

class ExosipCtxLock {
public:
	ExosipCtxLock(eXosip_t* ctx) :_ctx(ctx) {
		eXosip_lock(_ctx);
		//InfoL << "ctx lock!";
	}

	~ExosipCtxLock() {
		eXosip_unlock(_ctx);
		//InfoL << "ctx unlock!";
	}
private:
	eXosip_t* _ctx;
};

class Device {

	char deviceId[128] = { 0 };

	char videoChannelId[128] = { 0 };

	char server_sip_id[128] = { 0 };

	char  server_sip_realm[10] = { 0 };

	char server_ip[128] = { 0 };

	int server_port;

	char password[128] = { 0 };

	char  local_ip[128] = { 0 };

	int local_port;

	eXosip_t * sip_context = nullptr;

	NaluProvider* nalu_provider = nullptr;

	std::mutex _heartbeat_mutex;

	std::condition_variable _heartbeat_condition;

	std::mutex _mobile_position_mutex;

	std::condition_variable _mobile_postion_condition;

public:
	Device(const char * deviceId, const char * server_sip_id, const char * server_ip, int server_port, const char * password,
		NaluProvider* nalu_provider) {
		memcpy(this->deviceId, deviceId, strlen(deviceId));
		memcpy(this->videoChannelId, deviceId, strlen(deviceId));
		memcpy(this->server_sip_id, server_sip_id, strlen(server_sip_id));
		memcpy(server_sip_realm, server_sip_id, 10);
		memcpy(this->server_ip, server_ip, strlen(server_ip));
		this->server_port = server_port;
		memcpy(this->password, password, strlen(password));
		this->nalu_provider = nalu_provider;
	}
	void start_sip_client(int local_port);

	int list_index = 0;

	void set_callback(std::function<void(int index, Message msg)> callback);

	~Device();
private:

	void process_request();

	void send_response_ok(eXosip_event_t *evt);

	void send_response(eXosip_event_t *evt, osip_message_t * message);

	osip_message_t * create_request();
	
	void send_request(osip_message_t * request);

	bool register_success = false;

	bool is_tcp = false;

	const char * target_ip;

	int target_port;

	int listen_port;

	UDPClient* udp_client = nullptr;

	bool is_pushing = false;

	bool is_running = false;

	int callId = -1;

	int dialogId = -1;

	int mobile_postition_dialog_id = -1;

	int mobile_position_sn = 1;

	bool is_mobile_position_running = false;

	void push_task();

	void heartbeat_task();

	void create_mobile_position_task();

	void mobile_position_task();

	std::shared_ptr<std::thread> heartbeat_thread = nullptr;

	std::shared_ptr<std::thread> mobile_position_thread = nullptr;

	std::shared_ptr<std::thread> push_stream_thread = nullptr;

	std::shared_ptr<std::thread> sip_thread = nullptr;

	std::function<void(int index, Message msg)> callback;
	
};
