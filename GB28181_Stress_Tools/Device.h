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
#include <chrono>


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

	char _deviceId[128] = { 0 };

	char _videoChannelId[128] = { 0 };

	char _server_sip_id[128] = { 0 };

	char  _server_sip_realm[10] = { 0 };

	char _server_ip[128] = { 0 };

	int _server_port;

	char _password[128] = { 0 };

	char  _local_ip[128] = { 0 };

	int _local_port;

	eXosip_t *_sip_context = nullptr;

	NaluProvider *_nalu_provider = nullptr;
	uint32_t _ssrc = 0;


public:
	Device(const char *deviceId, const char *server_sip_id, const char *server_ip, int server_port, const char *password,
		NaluProvider* nalu_provider) {
		memcpy(this->_deviceId, deviceId, strlen(deviceId));
		memcpy(this->_videoChannelId, deviceId, strlen(deviceId));
		memcpy(this->_server_sip_id, server_sip_id, strlen(server_sip_id));
		memcpy(_server_sip_realm, server_sip_id, 10);
		memcpy(this->_server_ip, server_ip, strlen(server_ip));
		this->_server_port = server_port;
		memcpy(this->_password, password, strlen(password));
		this->_nalu_provider = nalu_provider;
	}

	void start_sip_client(int local_port);

	void stop_sip_client();

	int list_index = 0;

	void set_callback(std::function<void(int index, Message msg)> callback);

	~Device() {
		_is_pushing = false;
		_is_runing = false;

		if(_push_stream_thread)
			_push_stream_thread->join();

		if (_sip_process_thread)
			_sip_process_thread->join();

		if (_sip_context != NULL) {
			ExosipCtxLock lock(_sip_context);
			eXosip_quit(_sip_context);
			_sip_context = NULL;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

private:

	void process_request();

	void send_response_ok(eXosip_event_t *evt);

	void send_response(eXosip_event_t *evt, osip_message_t * message);

	osip_message_t * create_request();
	
	void send_request(osip_message_t * request);

	void push_task();

	void heartbeat_task();

	void unregister();


	void handler_message_new(eXosip_event_t *evt);
	void handler_register(eXosip_event_t *evt, bool status);
	void handler_call_ok(eXosip_event_t *evt);
	void handler_call_close(eXosip_event_t *evt);
	void handler_call_invite(eXosip_event_t *evt);

private:

	bool _register_success;

	bool _is_tcp;

	const char * _target_ip;

	int _target_port;

	int _listen_port;

	UDPClient* _udp_client = nullptr;

	bool _is_pushing;

	bool _is_runing;

	int _callId = -1;

	int _dialogId = -1;

	int _register_id;

	std::function<void(int index, Message msg)> _callback;

	std::shared_ptr<std::thread> _push_stream_thread;
	std::shared_ptr<std::thread> _sip_process_thread;
	std::shared_ptr<std::thread> _heartbeat_thread;
	
};
