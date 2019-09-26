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

class client {
public:
	client(const std::string &host, int port);
	virtual ~client();
	void send_message(const message &msg, message &response);
	bool get(const char *key, char *value, int64_t *timestamp);
	bool put(const char *key, const char *value, char *ov, int64_t *timestamp);
	bool get(std::string &key, std::vector<char> &value, int64_t &timestamp);	
	bool put(std::string &key, std::vector<char> &newvalue, std::vector<char> &oldvalue, int64_t &timestamp);
private:
	std::string host_;
	int port_;	
	int sock_;
};

#endif