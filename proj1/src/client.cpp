#include "client.h"


client::client(const std::string &host, int port) {
	host_ = host;
	port_ = port;
	sock_ = 0;
}


client::~client() {
	if (sock_) {
		close(sock_);
		sock_ = 0;
	}
}


void client::send_message(const message &msg, message &response) {
	DEBUG_PRINT("client::send_message() [begin]");

	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;

	inet_pton(AF_INET, host_.c_str(), &server_address.sin_addr);

	server_address.sin_port = htons(port_);

	if ((sock_ = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw exception("client::send_message() Error opening socket", errno);
	}

	if (connect(sock_, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		throw exception("client::send_message() Error connecting to end point", errno);
	}

	DEBUG_PRINT("client::send_message() send...");
	send(sock_, (void *) &msg, sizeof(message), 0);

	// get a response from the server
	int len = sizeof(message);
	int n = recv(sock_, (void *) &response, len, MSG_WAITALL);
	if (n<len) {
		throw exception("client::send_message() Invalid response size", errno);
	}

	close(sock_);
	sock_ = 0;

	DEBUG_PRINT("client::send_message() [end]");
}


bool client::get(const char *key, char *value, int *len, int64_t *timestamp) {
	DEBUG_PRINT("client::get() [begin]");

	try {
		message res;

		message msg(command::PUT);
		msg.set_key(key);
		
		send_message(msg, res);

		//if key not found
		if (msg.get_command()==command::NO_VAL) {
			DEBUG_PRINT("client::get() the key doesn't exist");
			return false;
		}
		else if (msg.get_command()!=command::OK) {
			DEBUG_PRINT("client::get() request failed!");
			return false;
		}

		//return the key value
		if (*len<res.get_value_size()) {
			throw exception("client::get() Invalid buffer size");
		}

		*timestamp = res.get_value_timestamp();
		memcpy(value, res.value(), res.get_value_size());
		return true;
	}
	catch (exception &ex) {
		DEBUG_PRINT("client::get() ERROR: %s", ex.what());
		return false;
	}
	return false;

	DEBUG_PRINT("client::get() [end]");
}

	 
bool client::put(const char *key, const char *value, int len, int64_t timestamp) {
	DEBUG_PRINT("client::put() [begin]");
	
	try {
		message res;

		message msg(command::PUT);
		msg.set_key(key);
		msg.set_value(value, len);
		msg.set_value_timestamp(timestamp);

		send_message(msg, res);
		if (res.get_command()!=command::OK) {
			DEBUG_PRINT("client::put() request failed!");
			return false;
		}
		return true;
	}
	catch (exception &ex) {
		DEBUG_PRINT("client::put() ERROR: %s", ex.what());
		return false;
	}

	DEBUG_PRINT("client::put() [end]");
}


bool client::get_meta(const char *key, char *value, int *len) {
	return false;
}


void client::put_meta(const char *key, const char *value) {

}
