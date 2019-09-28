#ifndef MESSAGE_H
#define MESSAGE_H

#include "lib739kv.h"
#include <chrono>
#include <cstdint>
#include <cctype>
#include <string>

#define MSG_SIZE 4096


enum class command: int32_t
{
	NONE = 0,
	OK = 1,
	CHK = 2,
	GET = 3,
	PUT = 4,
	NO_VAL = 5,
	ERROR = -1,
	SHUT_DOWN = 999
};


typedef struct header {
	int64_t id;
	command cmd;
	int32_t flag;
	int64_t param;
	int32_t key_size;
	int32_t value_size;
	int64_t value_timestamp;
} header_t;


class message {
public:

	message();
	message(command cmd);
	~message();

	void set_command(command cmd);
	command get_command() const;
	
	int64_t get_param() const;
	void set_param(int64_t param);
	
	int  get_key_size() const;
	void set_key(const char* key);
	
	int  get_value_size() const;
	void set_value(const char* value, int len);
	std::string get_value_string();
	void set_value_timestamp(int64_t ts);
	int64_t get_value_timestamp();
	
	//returns the pointer to the key string
	const char *key();
	
	//returns the pointers to the value
	const char *value();

	void clear();

private:
	void set_key_size(int len);
	void set_value_size(int len);

	header_t header_ = { };
	char payload[MSG_SIZE-sizeof(header_t)] = {0};
};
#endif