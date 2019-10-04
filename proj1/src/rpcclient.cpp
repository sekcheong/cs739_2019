#include <memory.h>
#include "rpcclient.h"
#include "exception.h"
#include <netinet/tcp.h>


void args_pack(char** list, char *buff) {
	int i=0;
	char *p = buff;
	while (list[i]!=0) {
		int len = strlen(list[i]);
		memcpy(p, list[i], len);
		p[len] = '|';
		p += (len+1);
		i++;
	}
	p[-1]=0;
}


void args_print(char **list) {
	int i=0;
	while (list[i]!=0) {
		printf("%d %s\n", i, list[i]);
		i++;
	}
}

rpc_client::rpc_client(int port) {
	host_ = "localhost";
	port_ = port;
	sock_ = 0;
}


rpc_client::~rpc_client() {
	if (sock_) {
		close(sock_);
		sock_ = 0;
	}
}


void rpc_client::send_message(const message &msg, message &response) {
	DEBUG_PRINT("rpc_client::send_message() [begin]");

	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;

	inet_pton(AF_INET, host_.c_str(), &server_address.sin_addr);

	server_address.sin_port = htons(port_);

	if ((sock_ = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw exception("rpc_client::send_message() Error opening socket", errno);
	}

	if (connect(sock_, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		throw exception("rpc_client::send_message() Error connecting to end point", errno);
	}

	int flag = 1; 
	setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	DEBUG_PRINT("rpc_client::send_message() send...");
	send(sock_, (void *) &msg, sizeof(message), 0);

	// get a response from the server
	int len = sizeof(message);
	int n = recv(sock_, (void *) &response, len, MSG_WAITALL);
	if (n<len) {
		throw exception("rpc_client::send_message() Invalid response size", errno);
	}

	close(sock_);
	//shutdown(sock_, SHUT_RDWR);
	sock_ = 0;

	DEBUG_PRINT("rpc_client::send_message() [end]");
}


int rpc_client::get(const char *key, char *value) {
	DEBUG_PRINT("rpc_client::get() [begin]");
	try {
		message res;
		message msg(command::GET);
		msg.set_key(key);
		
		send_message(msg, res);

		if (res.get_command()==command::OK) {
			DEBUG_PRINT("rpc_client::get() key[%s], value[%s]", key,  res.value());
			memcpy((void*)value, res.value(), res.get_value_size());
			return 0;
		}
		else if (res.get_command()==command::NO_VAL) {
			return 1;
		}
		else {
			return -1;
		}
	}
	catch (exception &ex) {
		DEBUG_PRINT("rpc_client::get() request failed! Error:%s", ex.what());
		return -1;
	}

	DEBUG_PRINT("rpc_client::get() [end]");
	return 1;
}


int rpc_client::put(const char *key, const char *value, const char *old_value) {
	DEBUG_PRINT("rpc_client::put() [begin]");

	try {
		message res;
		message msg(command::PUT);
		msg.set_key(key);
		int len = strlen(value);
		msg.set_value(value, len);

		send_message(msg, res);

		if (res.get_command()==command::OK) {
			DEBUG_PRINT("rpc_client::put() key[%s], value[%s], old_value[%s]", key,  msg.value(), res.value());
			memcpy((void*)old_value, res.value(), res.get_value_size());
			return 0;
		}
		else if (res.get_command()==command::NO_VAL) {
			DEBUG_PRINT("rpc_client::put() key[%s], value[%s] is new", key,  msg.value());
			return 1;
		}
		else {
			return -1;
		}
	}
	catch (exception &ex) {
		DEBUG_PRINT("rpc_client::put() request failed! Error:%s", ex.what());
		return -1;
	}

	DEBUG_PRINT("rpc_client::put() [end]");
	return 1;
}


int rpc_client::init(char **servers) {
	DEBUG_PRINT("rpc_client::init() [begin]");

	try {

		message res;
		message msg(command::INIT);

		char buffer[2048];
		args_pack(servers, buffer);
		int len = strlen(buffer);
		msg.set_value(buffer, len);

		send_message(msg, res);
		if (res.get_command()!=command::OK) {
			return -1;
		}
	}
	catch (exception &ex) {
		DEBUG_PRINT("rpc_client::init() request failed! Error:%s", ex.what());
		return -1;
	}

	DEBUG_PRINT("rpc_client::init() [end]");
	return 0;
}


int rpc_client::shutdown() {
	DEBUG_PRINT("rpc_client::shutdown() [begin]");

	try {
		message res;
		message msg(command::SHUT_DOWN);

		send_message(msg, res);
		if (res.get_command()!=command::OK) {
			return -1;
		}
	}
	catch (exception &ex) {
		DEBUG_PRINT("rpc_client::shutdown() request failed! Error:%s", ex.what());
		return -1;
	}

	DEBUG_PRINT("rpc_client::shutdown() [end]");
	return 0;
}