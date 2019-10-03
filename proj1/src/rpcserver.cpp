#include "rpcserver.h"
#include "message.h"
#include "exception.h"
#include <string.h>
#include <netinet/tcp.h>


void args_unpack(char **list, char *buff) {
	int j = 0;
	char *p = buff;
	int len = strlen(buff);
	list[j] = (char *) p;
	j = j + 1;
	for (int i=0; i<len-1; i++) {
		if (buff[i]=='|') {
			list[j] = (char *) &buff[i+1];
			buff[i] = 0;
			j++;
		}
	}
	list[j] = 0;
}


rpc_server::rpc_server(int port) {
	DEBUG_PRINT("rpc_server::rpc_server() [begin]");
	DEBUG_PRINT("rpc_server::rpc_server() port=%d", port);

	port_ = port;
	host_ = "localhost";

	DEBUG_PRINT("rpc_server::rpc_server() [end]");
}


rpc_server::~rpc_server() {
	DEBUG_PRINT("rpc_server::~rpc_server() [begin]");
	if (running_) {
		stop();
	}
	DEBUG_PRINT("rpc_server::~rpc_server() [end]");
}


void rpc_server::serve() {
	DEBUG_PRINT("rpc_server::rpc_server() [begin]");
    
    struct sockaddr_in serv_addr; 
 
 	sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) { 
        throw exception("rpc_server::serve() Error opening socket", errno);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_);

    if (bind(sockfd_, (struct sockaddr *) &serv_addr, sizeof(serv_addr))!=0 ) {
    	throw exception("rpc_server::serve() Error binding socket to port", errno);
	}
    
    if (listen(sockfd_, 5)!=0) {
    	throw exception("rpc_server::serve() Error listening to socket", errno);
    }

	running_ = true;
 	
 	listening_thread_ = new std::thread(&rpc_server::connection_handler, this);
 	processing_thread_ = new std::thread(&rpc_server::message_handler, this);

	DEBUG_PRINT("rpc_server::serve() [end]"); 	

}


void rpc_server::connection_handler() {
	DEBUG_PRINT("rpc_server::connection_handler() [begin]");
	
	int newsockfd;
	struct sockaddr_in cli_addr;
    while (running_) {
    	socklen_t clilen = sizeof(cli_addr);
    	newsockfd = accept(sockfd_, (struct sockaddr *) &cli_addr, &clilen);
    	if (!running_) break;
    	if (newsockfd < 0) {
    		throw exception("rpc_server::serve() Error accepting connection", errno);
    	}
    	conns_.enqueue(newsockfd);
	}

	DEBUG_PRINT("rpc_server::connection_handler() [end]");
}


// void print_value(const char *value, int len) {
// 	char *buff[MAX_VALUE_SIZE+1];
// 	memcpy(buff, value, len);
// 	buff[len] = 0;
// 	DEBUG_PRINT("rpc_server::value = [%d]", buff);
// }

void rpc_server::message_handler() {

	DEBUG_PRINT("rpc_server::message_handler() [begin]");

	while (running_) {

		int sockfd;

		if (conns_.dequeue_wait(sockfd)) {

			int flag = 1; 
			setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
			
			if (!running_) break;
			
			char buff[MAX_VALUE_SIZE+1];
			//int buff_len = sizeof(buff);	
			message msg;		

			int len = sizeof(msg);
			int n = recv(sockfd, (void *) &msg, len, MSG_WAITALL);

			if (n < len) {
				DEBUG_PRINT("rpc_server::message_handler() Error reading message");
			}

			switch (msg.get_command()) {
					
				case command::CHK: {
						message res;
						res.set_command(command::OK);
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				case command::OK: {						
						send(sockfd, (void *) &msg, sizeof(msg), 0);
					}
					break;

				case command::INIT: {
						message res(command::ERROR);
						DEBUG_PRINT("rpc_server::message_handler(): INIT");
						if (init_cb_) {
							char* args[128];
							args_unpack(args, (char*) msg.value());
							if (init_cb_(args)==0) {
								res.set_command(command::OK);
							}
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

    			case command::GET: {
    					message res(command::ERROR);
    					DEBUG_PRINT("rpc_server::message_handler(): GET: key=[%s]", msg.key());
						if (get_cb_) {
							int ret = get_cb_((char*)msg.key(), buff);
							if (ret==0) {
								DEBUG_PRINT("rpc_server::message_handler(): GET: value[%s]=[%s]", msg.key(), buff);
								res.set_command(command::OK);
								res.set_value(buff, strlen(buff));
							}
							else if (ret==1) {
								DEBUG_PRINT("rpc_server::message_handler(): GET: key[%s] doesn't exit!", msg.key());
								res.set_command(command::NO_VAL);
							}
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

    			case command::PUT: {
    					message res(command::ERROR);
    					DEBUG_PRINT("rpc_server::message_handler(): PUT: key[%s], value[%s]", msg.key(), msg.value());
						if (put_cb_) {
							// if (put_cb_((char*)msg.key(), (char*)msg.value(), buff)==1) {
							// 	res.set_command(command::OK);
							// 	res.set_value(buff, strlen(buff));
							// }
							int ret = put_cb_((char*)msg.key(), (char*)msg.value(), buff);
							if (ret==0) {
								DEBUG_PRINT("rpc_server::message_handler(): PUT: value[%s]=[%s] OK", msg.key(), buff);
								res.set_command(command::OK);
								res.set_value(buff, strlen(buff));
							}
							else if (ret==1) {
								DEBUG_PRINT("rpc_server::message_handler(): PUT: key[%s] doesn't exit!", msg.key());
								res.set_command(command::NO_VAL);
							}
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

    			case command::SHUT_DOWN: {
    					message res(command::ERROR);
    					DEBUG_PRINT("rpc_server::message_handler(): SHUT_DOWN");
						if (shutdown_cb_) {
							if (shutdown_cb_()==0) {
								res.set_command(command::OK);
							}
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
    				}
					break;

				default: {
						message res(command::ERROR);
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;
			}
		}

	}

	DEBUG_PRINT("rpc_server::message_handler() [end]");	
}


void rpc_server::stop() {

	DEBUG_PRINT("rpc_server::stop() [begin]");	

	//flag the system ready to shutdown
	running_ = false;

	//clear the connection queue and wake up all the blocking thread
	conns_.clear();

	//This will wake up any threads blocked on it, while keeping the file descriptor valid.
	shutdown(sockfd_, SHUT_RDWR);

	DEBUG_PRINT("rpc_server::stop() [end]");	
}


bool rpc_server::is_running() {
	return running_;
}

void rpc_server::set_init_callback(init_callback_t c) {
	init_cb_ = c;
}

void rpc_server::set_shutdown_callback(shutdown_callback_t c) {
	shutdown_cb_ = c;
}

void rpc_server::set_get_callback(get_callback_t c) {
	get_cb_ = c;
}

void rpc_server::set_put_callback(put_callback_t c) {
	put_cb_ = c;
}