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
#include "datastore.h"

class dsserver {

public:
	dsserver(const std::string &host, int port, const std::string &db_file);
	virtual ~dsserver();
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
	data_store *ds_ = nullptr;
	std::thread* listening_thread_ = nullptr;
	std::thread* processing_thread_ = nullptr;
};

#endif