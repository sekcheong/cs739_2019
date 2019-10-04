#include <iostream>
#include <memory>
#include <vector>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <chrono>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <tuple>
#include <iostream>
#include <string>
#include <stdexcept>
#include <unistd.h>

#include "lib739kv.h"

#define SERVER_LOC_SIZE 50
#define MAX_VAL_SIZE 2048
#define MAX_KEY_SIZE 128


#define KV_PAIRS 10000
#define KEY_LEN 32
#define VAL_LEN 128

#define WAIT_TIME_SEC 3

char rand_buff[5000];


int64_t os_timestamp() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}


std::vector<std::tuple<std::string, std::string>> make_key_value_pairs(int n) {
	std::vector<std::tuple<std::string, std::string>> kv;
	for (int i=0; i<n; i++) {
		rand_str(rand_buff, KEY_LEN);
		std::string key(rand_buff);
		rand_str(rand_buff, VAL_LEN);
		std::string value(rand_buff);
		kv.push_back(std::make_tuple(key, value));

	}
	return kv;
}


int main() {

	char *args[4];
	char buffer[MAX_VAL_SIZE+1];


	args[0] = (char *) "localhost:7390";
	args[1] = (char *) "localhost:7391";
	args[2] = (char *) "localhost:7392";
	args[3] = 0;


	auto keyvalues = make_key_value_pairs(KV_PAIRS);
	printf("** calling kv739_init()...\n");
	if (kv739_init(args)==0) {
		printf("** kv739_init() OK\n");
	}
	else {
		printf("** kv739_init() Failed!\n");
		return -1;
	}

	// insert the key, values
	printf("** Inserting %d key value pairs...\n", KV_PAIRS);
	
	int ret;
    auto t1 = os_timestamp();

	for (auto e:keyvalues) {
		std::string key = std::get<0>(e);
		std::string value = std::get<1>(e);
		//printf("key=%s, value=%s\n", key.c_str(), value.c_str());
		ret = kv739_put((char*) key.c_str(), (char*) value.c_str(), buffer);
	}

	auto t2 = os_timestamp();
	auto rate = ((double) keyvalues.size()) / ((double)(t2-t1) * (0.001));
    printf("== Insert throughput = %f keys/sec\n", rate );


    // Allow for eventual consistency 
    printf("** Wait %d seconds for eventual consistency to take place ...\n", WAIT_TIME_SEC);
    sleep(WAIT_TIME_SEC);


 	// check for key, values
	printf("** Retrieving key value pairs...\n");

    t1 = os_timestamp();
    int found = 0;
	for (auto e:keyvalues) {
		// printf("key=%s, value=%s\n", std::get<0>(e).c_str(), std::get<1>(e).c_str());
		std::string key = std::get<0>(e);
		std::string value = std::get<1>(e);

		ret = kv739_get( (char*) key.c_str(), buffer);

		if (ret==0) {
			if (strcmp((char*) value.c_str(), buffer) == 0) {
				found++;
			}
			else {
				printf("** key [%s] does not match value[%s]!\n", key.c_str(), value.c_str());
			}
		}
		else if (ret==1) {
			printf("** Key [%s] does not exist!\n",key.c_str());
		}
	}
	t2 = os_timestamp();

	printf("== Correct in reading back same value: %f percent\n",  (found*100) / ((double) keyvalues.size()));
	rate = ((double) keyvalues.size()) / ((double)(t2-t1) * (0.001));

	printf("== Read throughput = %f keys/sec\n", rate );


	printf("** calling kv739_shutdown()...\n");

	if (kv739_shutdown()==0) {
		printf("** kv739_shutdown() OK\n");
	}
	else {
		printf("** kv739_shutdown() Failed!\n");
	}

	return 0;
}
