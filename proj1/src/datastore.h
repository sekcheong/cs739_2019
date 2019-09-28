#ifndef DATASTORE_H
#define DATASTORE_H

#include <string>
#include <memory>
#include <vector>

#include "lib739kv.h"
#include "sqlite3.h"
#include "debug.h"
#include "exception.h"
#include "sqlstatement.h"


class data_store {
public:
	data_store();
	data_store(const char *filename);
	virtual ~data_store();

	virtual bool get(const char *key, char *value, int *len, int64_t *timestamp);
	virtual bool put(const char *key, const char *value, int len, int64_t timestamp);

	virtual bool get_meta(const char *key, char *value, int *len);
	virtual bool put_meta(const char *key, const char *value);
	
	//returns the latest timestamp, returns -1 if no value exist	
	virtual int64_t get_last_timestamp();
	
	//returns the earliest timestamp, returns -1 if no value exist
	virtual int64_t get_first_timestamp();

	virtual int64_t get_timestamp(const char *key);	
	
	virtual bool validate_key(const char* key);
	
	virtual bool validate_value(const char *value, int len);

	virtual bool get(const char *key, char *value, int *len) {
		int64_t ts;
		return get(key, value, len, &ts);
	}

	virtual bool put(const char *key, const char *value, int len) {
		int64_t ts = 0;
		return put(key, value, len, ts);
	}

	virtual bool put(const char *key, const char *value) {
		int len = strlen(value);
		return put(key, value, len);
	}

	static int64_t os_timestamp();

private:
	std::string filename_;
	sqlite3 *db_ = nullptr;
};
#endif