#ifndef RPCCLIENT_H
#define RPCCLIENT_H

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

class rpc_client {
public:
	rpc_client(int port);
	virtual ~rpc_client();
	void send_message(const message &msg, message &response);
	int get(const char *key, char *value);
	int put(const char *key, const char *value, const char *old_value);
	int init(char **servers);
	int shutdown();
private:
	std::string host_;
	int port_;
	int sock_;
};

#endif