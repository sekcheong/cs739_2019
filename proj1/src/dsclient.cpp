#include "dsclient.h"


dsclient::dsclient(const std::string &host, int port) {
	host_ = host;
	port_ = port;
	sock_ = 0;
}


dsclient::~dsclient() {
	if (sock_) {
		close(sock_);
		sock_ = 0;
	}
}


void dsclient::send_message(const message &msg, message &response) {
	DEBUG_PRINT("dsclient::send_message() [begin]");

	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;

	inet_pton(AF_INET, host_.c_str(), &server_address.sin_addr);

	server_address.sin_port = htons(port_);

	if ((sock_ = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw exception("dsclient::send_message() Error opening socket", errno);
	}

	if (connect(sock_, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		throw exception("dsclient::send_message() Error connecting to end point", errno);
	}

	DEBUG_PRINT("dsclient::send_message() send...");
	send(sock_, (void *) &msg, sizeof(message), 0);

	// get a response from the server
	int len = sizeof(message);
	int n = recv(sock_, (void *) &response, len, MSG_WAITALL);
	if (n<len) {
		throw exception("dsclient::send_message() Invalid response size", errno);
	}

	close(sock_);
	sock_ = 0;

	DEBUG_PRINT("dsclient::send_message() [end]");
}


bool dsclient::get(const char *key, char *value, int *len, int64_t *timestamp) {
	DEBUG_PRINT("dsclient::get() [begin]");

	if (!data_store::validate_key(key)) {
		throw exception("dsclient::get() invalid key!");
	}

	message res;

	message msg(command::GET);
	msg.set_key(key);
	
	send_message(msg, res);

	//if key not found
	if (res.get_command()==command::NO_VAL) {
		DEBUG_PRINT("dsclient::get() the key doesn't exist");
		return false;
	}
	else if (res.get_command()!=command::OK) {
		throw exception("dsclient::get() request failed!");
	}

	//return the key value
	if (*len<res.get_value_size()) {
		throw exception("dsclient::get() Invalid buffer size");
	}

	*timestamp = res.get_value_timestamp();
	memcpy(value, res.value(), res.get_value_size());

	DEBUG_PRINT("dsclient::get() [end]");

	return true;
}

	 
void dsclient::put(const char *key, const char *value, int len, int64_t timestamp) {
	DEBUG_PRINT("dsclient::put() [begin]");

	if (!data_store::validate_key(key)) {
		throw exception("dsclient::put() invalid key!");
	}

	if (!data_store::validate_value(value, len)) {
		throw exception("dsclient::put() invalid value!");
	}

	message res;

	message msg(command::PUT);
	msg.set_key(key);
	msg.set_value(value, len);
	msg.set_value_timestamp(timestamp);

	send_message(msg, res);
	if (res.get_command()!=command::OK) {
		throw exception("dsclient::put() request failed!");
	}

	DEBUG_PRINT("dsclient::put() [end]");
}


bool dsclient::get_meta(const char *key, char *value, int *len) {
	DEBUG_PRINT("dsclient::get_meta() [begin]");

	message res;

	message msg(command::GET_META);
	msg.set_key(key);
	
	send_message(msg, res);

	//if key not found
	if (res.get_command()==command::NO_VAL) {
		throw exception("dsclient::get_meta() the key doesn't exist");
	}
	else if (res.get_command()!=command::OK) {
		throw exception("dsclient::get_meta() request failed!");
	}

	//return the key value
	if (*len<res.get_value_size()) {
		throw exception("dsclient::get_meta() Invalid buffer size");
	}

	memset((void*) value, 0, *len);
	memcpy(value, res.value(), res.get_value_size());

	DEBUG_PRINT("dsclient::get_meta() [end]");

	return true;
}


void dsclient::put_meta(const char *key, const char *value) {
	DEBUG_PRINT("dsclient::put_meta() [begin]");

	message res;

	message msg(command::PUT_META);
	msg.set_key(key);
	int len = strlen(value);
	msg.set_value(value, len);
	msg.set_value_timestamp(0);

	send_message(msg, res);
	if (res.get_command()!=command::OK) {
		throw exception("dsclient::put_meta() request failed");
	}

	DEBUG_PRINT("dsclient::put_meta() [end]");
}


int64_t dsclient::get_last_timestamp()  {
	DEBUG_PRINT("dsclient::get_last_timestamp() [begin]");
	message res;
	message msg(command::GET_LAST_TS);
	send_message(msg, res);

	if (res.get_command()!=command::OK) {
		throw exception("dsclient::get_last_timestamp() request failed");
	}
	DEBUG_PRINT("dsclient::get_last_timestamp() [end]");
	return res.get_value_timestamp();
}


int64_t dsclient::get_first_timestamp() {
	DEBUG_PRINT("dsclient::get_first_timestamp() [begin]");
	message res;
	message msg(command::GET_FIRST_TS);
	send_message(msg, res);

	if (res.get_command()!=command::OK) {
		throw exception("dsclient::get_first_timestamp() request failed!");
		return -1;
	}
	DEBUG_PRINT("dsclient::get_first_timestamp() [end]");
	return res.get_value_timestamp();
}


int64_t dsclient::get_timestamp(const char *key) {
	DEBUG_PRINT("dsclient::get_timestamp() [begin]");
	message res;
	message msg(command::GET_TS);
	msg.set_key(key);
	send_message(msg, res);

	if (res.get_command()!=command::OK) {
		throw exception("dsclient::get_timestamp() request failed!");
		return -1;
	}
	DEBUG_PRINT("dsclient::get_timestamp() [end]");
	return res.get_value_timestamp();
}


void dsclient::shutdown_server() {
	DEBUG_PRINT("dsclient::shutdown_server() [begin]");
	message res;
	message msg(command::SHUT_DOWN);
	send_message(msg, res);
	if (res.get_command()!=command::OK) {
		throw exception("dsclient::shutdown_server() request failed!");
	}
	DEBUG_PRINT("dsclient::shutdown_server() [end]");
}
