#include "kvproxy.h"

kv_proxy::kv_proxy() {

}

kv_proxy::~kv_proxy() {
	init_cb_ = nullptr;
	shutdown_cb_ = nullptr;
	get_cb_ = nullptr;
	put_cb_ =nullptr;
}

int kv_proxy::init(char** server_list) {
	if (!init_cb_) return -1; 
	return init_cb_(server_list);
}

int kv_proxy::shutdown(void) {
	if (!shutdown_cb_) return -1;
	return shutdown_cb_();
}

int kv_proxy::get(char* key, char* value) {
	if (!get_cb_) return -1;
	return get_cb_(key, value);
}

int kv_proxy::put(char* key, char* value, char* old_value) {
	if (!put_cb_) return -1;
	return put_cb_(key, value, old_value);
}

void kv_proxy::set_init_callback(init_callback_t c) {
	init_cb_ = c;
}

void kv_proxy::set_shutdown_callback(shutdown_callback_t c) {
	shutdown_cb_ = c;
}

void kv_proxy::set_get_callback(get_callback_t c) {
	get_cb_ = c;
}

void kv_proxy::set_put_callback(put_callback_t c) {
	put_cb_ = c;
}