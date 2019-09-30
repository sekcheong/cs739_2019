#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>

#include "message.h"
#include "exception.h"
#include "debug.h"
#include "datastore.h"

class dsclient {
public:
	dsclient(const std::string &host, int port);
	virtual ~dsclient();
	void send_message(const message &msg, message &response);
	bool get(const char *key, char *value, int *len, int64_t *timestamp);
	void put(const char *key, const char *value, int len, int64_t timestamp);
	void put(const char *key, const char *value, int len) {
		 put(key, value, len, 0);
	}
	bool get_meta(const char *key, char *value, int *len);
	void put_meta(const char *key, const char *value);
	int64_t get_last_timestamp();
	int64_t get_first_timestamp();
	int64_t get_timestamp(const char *key);	
	void shutdown_server();
private:
	std::string host_;
	int port_;	
	int sock_;
};

#endif