#include "dsserver.h"
#include "message.h"
#include "exception.h"


dsserver::dsserver(const std::string &host, int port, const std::string &db_file) {
	DEBUG_PRINT("dsserver::dsserver() [begin]");
	DEBUG_PRINT("dsserver::dsserver() host=%s, port=%d", host.c_str(), port);

	port_ = port;
	host_ = host;
	db_file_ = db_file;	
	ds_ = new data_store(db_file.c_str());

	DEBUG_PRINT("dsserver::dsserver() [end]");
}


dsserver::~dsserver() {
	DEBUG_PRINT("dsserver::~dsserver() [begin]");
	if (running_) {
		stop();
	}

	if (ds_) {
		delete ds_;
		ds_ = nullptr;
	}
	DEBUG_PRINT("dsserver::~dsserver() [end]");
}


void dsserver::serve() {
	DEBUG_PRINT("dsserver::dsserver() [begin]");
    
    struct sockaddr_in serv_addr; 
 
 	sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) { 
        throw exception("dsserver::serve() Error opening socket", errno);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_);

    if (bind(sockfd_, (struct sockaddr *) &serv_addr, sizeof(serv_addr))!=0 ) {
    	throw exception("dsserver::serve() Error binding socket to port", errno);
	}
    
    if (listen(sockfd_, 5)!=0) {
    	throw exception("dsserver::serve() Error listening to socket", errno);
    }

	running_ = true;
 	
 	listening_thread_ = new std::thread(&dsserver::connection_handler, this);
 	processing_thread_ = new std::thread(&dsserver::message_handler, this);

	DEBUG_PRINT("dsserver::serve() [end]"); 	

}


void dsserver::connection_handler() {
	DEBUG_PRINT("dsserver::connection_handler() [begin]");
	
	int newsockfd;
	struct sockaddr_in cli_addr;
    while (running_) {
    	socklen_t clilen = sizeof(cli_addr);
    	newsockfd = accept(sockfd_, (struct sockaddr *) &cli_addr, &clilen);
    	if (!running_) break;
    	if (newsockfd < 0) {
    		throw exception("dsserver::serve() Error accepting connection", errno);
    	}
    	conns_.enqueue(newsockfd);
	}

	DEBUG_PRINT("dsserver::connection_handler() [end]");
}


void dsserver::message_handler() {

	DEBUG_PRINT("dsserver::message_handler() [begin]");

	while (running_) {

		int sockfd;

		if (conns_.dequeue_wait(sockfd)) {
			
			if (!running_) break;
			
			char buff[MAX_VALUE_SIZE];
			int buff_len = sizeof(buff);	
			message msg;		

			int len = sizeof(msg);
			int n = recv(sockfd, (void *) &msg, len, MSG_WAITALL);

			if (n < len) {
				DEBUG_PRINT("dsserver::message_handler() Error reading message");
			}

			switch (msg.get_command()) {
					
				case command::CHK: {
						DEBUG_PRINT("dsserver::message_handler(): key=%s", msg.key());
						message res;
						res.set_command(command::OK);
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				case command::OK: {						
						send(sockfd, (void *) &msg, sizeof(msg), 0);
					}
					break;

    			case command::GET: {
						DEBUG_PRINT("dsserver::message_handler(): GET: key=%s", msg.key());
						message res;
						int64_t ts = 0;
						try {
							buff_len = sizeof(buff);
							if (ds_->get(msg.key(), buff, &buff_len, &ts)) {
								res.set_value(buff, buff_len);
								res.set_value_timestamp(ts);
								res.set_command(command::OK);
								DEBUG_PRINT("dsserver::message_handler(): GET: OK");
							}
							else {
								DEBUG_PRINT("dsserver::message_handler(): GET: NoVal");
								res.set_command(command::NO_VAL);
							}

						}
						catch (exception &ex) {
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): GET: Error %s", ex.what());
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

    			case command::PUT: {

						DEBUG_PRINT("dsserver::message_handler(): PUT: key=%s, value=%s, timestamp=%d", msg.key(), msg.get_value_string().c_str(), msg.get_value_timestamp());
						message res;
						try {
							buff_len = sizeof(buff);
							int64_t ts = msg.get_value_timestamp();
							if (ds_->put(msg.key(), msg.value(), msg.get_value_size(), ts)) {
								res.set_value(buff, buff_len);
								res.set_value_timestamp(ts);
								res.set_command(command::OK);
							}
							else {
								res.clear();
								res.set_command(command::ERROR);
								DEBUG_PRINT("dsserver::message_handler(): PUT: Returns FALSE!");
							}
						}
						catch (exception &ex) {
							res.clear();
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): PUT: Error %s", ex.what());
						}

						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

    			case command::SHUT_DOWN: {
    					message res;
						msg.set_command(command::OK);
						send(sockfd, (void *) &msg, sizeof(message), 0);
    					stop();
    				}
					break;

				case command::GET_META: {
						DEBUG_PRINT("dsserver::message_handler(): GET_META: key=%s", msg.key());
						message res;
						try {
							buff_len = sizeof(buff);
							if (ds_->get_meta(msg.key(), buff, &buff_len)) {
								res.set_value(buff, buff_len);
								res.set_command(command::OK);
								DEBUG_PRINT("dsserver::message_handler(): GET_META: OK");
							}
							else {
								DEBUG_PRINT("dsserver::message_handler(): GET_META: NoVal");
								res.set_command(command::NO_VAL);
							}

						}
						catch (exception &ex) {
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): GET_META: Error %s", ex.what());
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				case command::PUT_META:{
						DEBUG_PRINT("dsserver::message_handler(): PUT_META: key=%s, value=%s", msg.key(), msg.get_value_string().c_str());
						message res;
						try {
							buff_len = sizeof(buff);
							int64_t ts = msg.get_value_timestamp();
							if (ds_->put_meta(msg.key(), msg.value())) {
								res.set_value(buff, buff_len);
								res.set_value_timestamp(ts);
								res.set_command(command::OK);
							}
							else {
								res.clear();
								res.set_command(command::ERROR);
								DEBUG_PRINT("dsserver::message_handler(): PUT_META: Returns FALSE!");
							}
						}
						catch (exception &ex) {
							res.clear();
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): PUT_META: Error %s", ex.what());
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				case command::GET_TS:{
						DEBUG_PRINT("dsserver::message_handler(): GET_TS");
						message res;
						try {
							buff_len = sizeof(buff);
							int64_t ts = ds_->get_timestamp(msg.key());
							res.set_value_timestamp(ts);
							res.set_command(command::OK);
						}
						catch (exception &ex) {
							res.clear();
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): GET_TS: Error %s", ex.what());
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				case command::GET_LAST_TS:{
						DEBUG_PRINT("dsserver::message_handler(): GET_LAST_TS");
						message res;
						try {
							buff_len = sizeof(buff);
							int64_t ts = ds_->get_last_timestamp();
							res.set_value_timestamp(ts);
							res.set_command(command::OK);
						}
						catch (exception &ex) {
							res.clear();
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): GET_LAST_TS: Error %s", ex.what());
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				case command::GET_FIRST_TS:{
						DEBUG_PRINT("dsserver::message_handler(): GET_FIRST_TS");
						message res;
						try {
							buff_len = sizeof(buff);
							int64_t ts = ds_->get_first_timestamp();
							res.set_value_timestamp(ts);
							res.set_command(command::OK);
						}
						catch (exception &ex) {
							res.clear();
							res.set_command(command::ERROR);
							DEBUG_PRINT("dsserver::message_handler(): GET_FIRST_TS: Error %s", ex.what());
						}
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;

				default: {
						message res;
						res.clear();
						res.set_command(command::OK);
						send(sockfd, (void *) &res, sizeof(message), 0);
					}
					break;
			}
		}

	}

	DEBUG_PRINT("dsserver::message_handler() [end]");	
}


void dsserver::stop() {

	DEBUG_PRINT("dsserver::stop() [begin]");	

	//flag the system ready to shutdown
	running_ = false;

	//clear the connection queue and wake up all the blocking thread
	conns_.clear();

	//This will wake up any threads blocked on it, while keeping the file descriptor valid.
	shutdown(sockfd_, SHUT_RDWR);

	DEBUG_PRINT("dsserver::stop() [end]");	
}


bool dsserver::is_running() {
	return running_;
}


// #include <sys/ipc.h> 
// #include <sys/shm.h> 
// #include <stdio.h> 
// using namespace std; 

// int main() 
// { 
// 	// ftok to generate unique key 
// 	key_t key = ftok("shmfile",65); 

// 	// shmget returns an identifier in shmid 
// 	int shmid = shmget(key,1024,0666|IPC_CREAT); 

// 	// shmat to attach to shared memory 
// 	char *str = (char*) shmat(shmid,(void*)0,0); 

// 	cout<<"Write Data : "; 
// 	gets(str); 

// 	printf("Data written in memory: %s\n",str); 
	
// 	//detach from shared memory 
// 	shmdt(str); 

// 	return 0; 
// }


// #include <iostream> 
// #include <sys/ipc.h> 
// #include <sys/shm.h> 
// #include <stdio.h> 
// using namespace std; 
  
// int main() 
// { 
//     // ftok to generate unique key 
//     key_t key = ftok("shmfile",65); 
  
//     // shmget returns an identifier in shmid 
//     int shmid = shmget(key,1024,0666|IPC_CREAT); 
  
//     // shmat to attach to shared memory 
//     char *str = (char*) shmat(shmid,(void*)0,0); 
  
//     printf("Data read from memory: %s\n",str); 
      
//     //detach from shared memory  
//     shmdt(str); 
    
//     // destroy the shared memory 
//     shmctl(shmid,IPC_RMID,NULL); 
     
//     return 0; 
// } 
