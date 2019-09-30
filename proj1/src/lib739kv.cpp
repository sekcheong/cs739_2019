#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "lib739kv.h"
#include "debug.h"
//#include "server.h"

static kv_proxy *proxy_ = nullptr;

int kv739_init(char** server_list) {
	DEBUG_PRINT("kv739_init()");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	return p->init(server_list);
}

int kv739_shutdown(void) {
	DEBUG_PRINT("kv739_shutdown()");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	return p->shutdown();
}

int kv739_get(char* key, char* value) {
	DEBUG_PRINT("kv739_get()");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	return p->get(key, value);
}

int kv739_put(char* key, char* value, char* old_value) {
	DEBUG_PRINT("kv739_put()");  
	auto p = kv739_get_proxy();
	if (!p) return -1;
	return p->put(key, value, old_value);
}

void kv739_set_proxy(kv_proxy *p) {
	proxy_ = p;
}

kv_proxy *kv739_get_proxy() {
	return proxy_;
}