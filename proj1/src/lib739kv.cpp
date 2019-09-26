#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "lib739kv.h"
#include "server.h"

int kv739_init(char** server_list) {
	DEBUG_PRINT("kv739_init() [begin]");

	//init code goes here

	if (!server_list) return -1;	
	int i = 0;
	while (server_list[i]) {
		DEBUG_PRINT("kv739_init()  server %d: %s", i, server_list[i]);
		i++;
	}

	DEBUG_PRINT("kv739_init() [end]");
	return 1;
}

int kv739_shutdown(void) {
	DEBUG_PRINT("kv739_shutdown() [begin]");

	//shutdown code goes here

	DEBUG_PRINT("kv739_shutdown() [end]");
	return 1;
}

int kv739_get(char* key, char* value) {
	DEBUG_PRINT("kv739_get() [begin]");

	DEBUG_PRINT("kv739_get() key=%s", key);

	DEBUG_PRINT("kv739_get() &value=%08x", value);

	strcpy(value, "hello");

	DEBUG_PRINT("kv739_get() [end]");
	return 1;
}

int kv739_put(char* key, char* value, char* old_value) {
	DEBUG_PRINT("kv739_put() [begin]");	

	DEBUG_PRINT("kv739_put() key=%s", key);
	DEBUG_PRINT("kv739_put() value=%s", value);
	DEBUG_PRINT("kv739_put() &value=%08x", old_value);

	strcpy(old_value, "world");

	DEBUG_PRINT("kv739_put() [end]");
	return 1;
}