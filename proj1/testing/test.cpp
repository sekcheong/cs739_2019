#include <iostream>
#include <memory>
#include <vector>
#include "lib739kv.h"
#include "datastore.h"
#include "message.h"

std::string vec2str(std::vector<char> v) {
	std::string s(v.begin(), v.end());
	return s;
}


std::vector<char> str2vec(std::string &s) {
	std::vector<char> v(s.begin(), s.end());
	return v;
}


std::vector<char> str2vec(const char *s) {
	std::vector<char> v(s, s + strlen(s));
	return v;
}

void test_kv_store() {
	DEBUG_PRINT("  SQLite3 Version: %s\n", sqlite3_libversion()); 
	
	data_store ds("test.db");

	try {
		int64_t ts;			
		char ov[2048];
		int ov_len = sizeof(ov);	

		const char *key = "foo";
		const char *nv = "What's up doc?";
		
		ds.put(key, nv, strlen(nv), ov, &ov_len, &ts);
		std::cout << "key      : " << key << std::endl;		
		//if we have an old value
		if (ov_len>-1) {
			ov[ov_len] = 0;
			std::cout << "old value: " << ov << std::endl;
			std::cout << "time     : " << ts << std::endl;
		}
	}
	catch (exception &ex) {
		std::cerr << ex.what()  << " error_code: " << ex.code() << std::endl;
	}

	
	try {
		ds.put("foo", "Hello World!");
		ds.put("foo", "Goodbye!");
	}
	catch (exception &ex) {
		std::cerr << ex.what()  << " error_code: " << ex.code() << std::endl;
	}

	try {
		int64_t ts;			
		char buff[2048];
		int len = sizeof(buff);	
		const char *key = "foo";

		std::cout << "key      : " << key << std::endl;
		if (ds.get(key, buff, &len, &ts)) {
			buff[len]=0;
			std::cout << "value    : " << buff << std::endl;
			std::cout << "time     : " << ts << std::endl;	
		}
		else {
			std::cout << "Key doesn't exist!" << std::endl;
		}

	}
	catch (exception &ex) {
		std::cerr << ex.what()  << " error_code: " << ex.code() << std::endl;
	}

	try {
		int64_t ts;			
		char ov[2048];
		int ov_len = sizeof(ov);	

		const char *key = "foo";
		const char *nv = "What's up doc?";
		
		ds.put("foo", nv, strlen(nv), ov, &ov_len, &ts);
		std::cout << "key      : " << key << std::endl;

		//if we have an old value
		if (ov_len>-1) {
			ov[ov_len] = 0;
			std::cout << "old value: " << ov << std::endl;
			std::cout << "time     : " << ts << std::endl;
		}
	}
	catch (exception &ex) {
		std::cerr << ex.what()  << " error_code: " << ex.code() << std::endl;
	}

	std::cout << "first timestamp: " << ds.get_first_timestamp() << std::endl;
	std::cout << "last timestamp : " << ds.get_last_timestamp() << std::endl;
}


void test_message() {
	DEBUG_PRINT("test_message() [begin]");
	DEBUG_PRINT("test_message() size=%d", sizeof(message));
	message msg(command::OK);
	char v[] = {'a', 'b', 'c', 'd', 'e', 'f'};
	
	msg.set_key("foo");
	
	msg.set_value(v, sizeof(v));
	
	std::cout << "key=" << msg.key() << std::endl;

	std::cout << "value len=" << msg.get_value_size() << std::endl;

	for (int i=0; i<msg.get_value_size(); i++) {
		auto *val = msg.value();
		std::cout << val[i] << " ";
	}
	std::cout << std::endl;

	DEBUG_PRINT("test_message() [end]");
}





int main() {
	DEBUG_PRINT("main() [begin]");
	
	kv739_init(0);
	kv739_get(0, 0);
	kv739_put(0, 0, 0);	
	kv739_shutdown();
	
	test_kv_store();
	test_message();

	DEBUG_PRINT("main() [end]");



	return 0;
}