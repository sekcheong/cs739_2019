#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "lib739kv.h"
#include "debug.h"
//#include "server.h"

static kv_proxy *proxy_ = 0;

int kv739_init(char** server_list) {
	DEBUG_PRINT("kv739_init() [begin]");  
	auto p = kv739_get_proxy();
	printf("%x\n",p);
	if (!p) return -1;
	DEBUG_PRINT("kv739_init() [end]");  
	return p->init(server_list);
}

int kv739_shutdown(void) {
	DEBUG_PRINT("kv739_shutdown() [begin]");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	DEBUG_PRINT("kv739_shutdown() [end]");  
	return p->shutdown();
}

int kv739_get(char* key, char* value) {
	DEBUG_PRINT("kv739_get() [begin]");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	DEBUG_PRINT("kv739_get() [end]");  
	return p->get(key, value);
}

int kv739_put(char* key, char* value, char* old_value) {
	DEBUG_PRINT("kv739_put() [begin]");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	DEBUG_PRINT("kv739_put() [end]"); 
	return p->put(key, value, old_value);
}

void kv739_set_proxy(kv_proxy *p) {
	DEBUG_PRINT("kv739_set_proxy() [begin]"); 
	proxy_ = p;
	DEBUG_PRINT("kv739_set_proxy() [end]"); 
}

kv_proxy *kv739_get_proxy() {
	return proxy_;
}