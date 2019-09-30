#ifndef KVPROXY_H
#define KVPROXY_H

#include <memory>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*init_callback_t)(char** server_list);
typedef int (*shutdown_callback_t)(void);
typedef int (*get_callback_t)(char* key, char* value);
typedef int (*put_callback_t)(char* key, char* value, char* old_value);

class kv_proxy {
public:
	kv_proxy();
	virtual ~kv_proxy();
	int init(char** server_list);
	int shutdown(void);
	int get(char* key, char* value);
	int put(char* key, char* value, char* old_value);

	void set_init_callback(init_callback_t c);
	void set_shutdown_callback(shutdown_callback_t c);
	void set_get_callback(get_callback_t c);
	void set_put_callback(put_callback_t c);

private:
	init_callback_t init_cb_ = nullptr;
	shutdown_callback_t shutdown_cb_ = nullptr;
	get_callback_t get_cb_ = nullptr;
	put_callback_t put_cb_ =nullptr;
};

#endif