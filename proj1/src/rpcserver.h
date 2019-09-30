#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h> 
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "debug.h"
#include "queue.h"

typedef int (*init_callback_t)(char** server_list);
typedef int (*shutdown_callback_t)(void);
typedef int (*get_callback_t)(char* key, char* value);
typedef int (*put_callback_t)(char* key, char* value, char* old_value);

class rpc_server {

public:
	rpc_server(int port);
	virtual ~rpc_server();
	void serve();
	void connection_handler();
	void message_handler();
	void stop();
	bool is_running();
	
private:
	int port_;
	std::string host_;
	std::string db_file_;
	int sockfd_;
	queue<int> conns_;
	bool running_;
	std::thread* listening_thread_ = nullptr;
	std::thread* processing_thread_ = nullptr;
};

#endif