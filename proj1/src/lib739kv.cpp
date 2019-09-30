#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "debug.h"
#include "lib739kv.h"
#include "rpcclient.h"

static rpc_client *client_ = 0;

rpc_client *kv739_get_client() {
	try {
		if (!client_) {
			client_ = new rpc_client(RPC_PORT);
		}
	}
	catch (...) {

	}
	return client_;
}

int kv739_init(char** server_list) {
	DEBUG_PRINT("kv739_init() [begin]");  
	auto p = kv739_get_client();
	if (!p) return -1;
	DEBUG_PRINT("kv739_init() [end]");  
	return p->init(server_list);
}

int kv739_shutdown(void) {
	DEBUG_PRINT("kv739_shutdown() [begin]");  
	auto p = kv739_get_client();
	if (!p) return -1;
	DEBUG_PRINT("kv739_shutdown() [end]");  
	return p->shutdown();
}

int kv739_get(char* key, char* value) {
	DEBUG_PRINT("kv739_get() [begin]");  
	auto p = kv739_get_client();
	if (!p) return -1;
	DEBUG_PRINT("kv739_get() [end]");  
	return p->get(key, value);
}

int kv739_put(char* key, char* value, char* old_value) {
	DEBUG_PRINT("kv739_put() [begin]");  
	auto p = kv739_get_client();
	if (!p) return -1;
	DEBUG_PRINT("kv739_put() [end]"); 
	return p->put(key, value, old_value);
}