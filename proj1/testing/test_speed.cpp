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


#define WAIT_TIME_SEC 3
#define KV_PAIRS 100
#define KEY_LEN 32
#define VAL_LEN 128

char rand_buff[5000];

// #ifndef KVSTORE_SIZE
//#define KVSTORE_SIZE 100000 // In terms of number of keys


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


// void rand_string(char *str, size_t size) {
//     const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
//     if (!size) return;
//         for (size_t n = 0; n < size-1; n++) {
//             int key = rand() % (int) (sizeof charset - 1);
//             str[n] = charset[key];
//         }
//         str[size] = 0;
//     return str;
// }

// float get_time_elapsed_sec(struct timeval tv1, struct timeval tv2) {
//     struct timeval tvdiff = { tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec };
//     if (tvdiff.tv_usec < 0) { tvdiff.tv_usec += 1000000; tvdiff.tv_sec -= 1; }
//     return tvdiff.tv_sec + tvdiff.tv_usec/(1000.0*1000.0);
// }

// int main(int argc, char *argv[])
// {
//     Py_Initialize();
//     initclient();
   
//     int num_servers = argc-1; 
//     // Test code starts here
//     char **servers;
//     servers = (char**)malloc(num_servers*sizeof(char*));

//     for(int i=0;i<num_servers;i++) {
// 	servers[i] = (char*)malloc(SERVER_LOC_SIZE*sizeof(char));
// 	strcpy(servers[i], argv[i+1]);
//     }

//     int ret = kv739_init(servers, num_servers);
//     printf("Kv_init %d\n", ret);

//     if(ret == -1){
// 	    exit(0);
//     }
    
//     // Now generate KVSTORE_SIZE random keys
//     char **keys;
//     keys = (char**) malloc(KVSTORE_SIZE*sizeof(char*));
//     for(int i=0;i<KVSTORE_SIZE;i++) {
//         keys[i] = (char*)malloc(MAX_VAL_SIZE*sizeof(char));
// 	rand_string(keys[i], 128);
//     }

//     // Put seed values for 100000 random keys
//     char **values;
//     values = (char**) malloc(KVSTORE_SIZE*sizeof(char*));
//     for(int i=0;i<KVSTORE_SIZE;i++) {
// 	values[i] = (char*)malloc(MAX_VAL_SIZE*sizeof(char));
//         rand_string(values[i], 512);
//     }

    // // Insert the KV pairs
    // char *old_val;
    // old_val = (char*) malloc(MAX_VAL_SIZE * sizeof(char));
    // for(int i=0;i<KVSTORE_SIZE;i++) {
    //    ret = kv739_put(keys[i], values[i], old_val);
    //    //assert(ret == 1); // there should be no failure
    //    //assert(old_val[0] == '\0'); // old value should be NULL
    // }

//     // Allow for eventual consistency to play out its magic
//     usleep(SLEEP_TIME_US);

//     // Check if we are able to read back same values
//     struct timeval tv1, tv2;
//     int errors = 0;
//     int num_iterations = 10;
//     float tot = 0;
//     while(num_iterations > 0) {
//        errors = 0;
//        gettimeofday(&tv1, NULL);
//        for(int i=0;i<KVSTORE_SIZE;i++) {
//            ret = kv739_get(keys[i], old_val);
//            assert(ret == 0); // there should be no failure
//            if(strcmp(old_val, values[i]) != 0) {
//                errors++;
//            }
//        }
//        gettimeofday(&tv2, NULL);
//        num_iterations--;
//        printf("Error in reading back same value: %f percent\n", (errors*100.0)/KVSTORE_SIZE);
//        printf("Read throughput = %f keys/sec\n", (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2));
//        tot = tot + (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2);
//     }
//     printf("Avg. Read throughput = %f keys/sec\n", tot/10);

//     // Now test atomic put and get
//     // Check if we are able to read back same values
//     num_iterations = 10;
//     tot = 0;
//     while(num_iterations > 0) {
//     	errors = 0;
//     	char *val;
//     	val = (char*) malloc(MAX_VAL_SIZE * sizeof(char));
// 	gettimeofday(&tv1, NULL);
//     	for(int i=0;i<KVSTORE_SIZE;i++) {
//        	    rand_string(val, 512);
//             ret = kv739_put(keys[i], val, old_val);
//             assert(ret == 0); // there should be no failure
//             if(strcmp(old_val, values[i]) != 0) {
// 	       printf("old: %s\nnew: %s\n", old_val, values[i]);
//                errors++;
//             }
//             strcpy(values[i], val);
//         }
// 	gettimeofday(&tv2, NULL);
// 	num_iterations--;
//         printf("Errors in putting value for same key: %f percent\n", (errors*100.0)/KVSTORE_SIZE);
// 	printf("Write throughput = %f keys/sec\n", (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2));
// 	tot = tot + (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2);
// 	usleep(SLEEP_TIME_US);
//     }
//     printf("Avg. Write throughput = %f\n keys/sec", tot/10);
//     Py_Finalize();
//     return 0;
// }



// std::vector<std::string> ;

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

	printf("** Inserting %d key value pairs...\n", KV_PAIRS);
	// insert the key, values
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


	printf("** Retrieving key value pairs...\n");
    // check for key, values
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


    // char str[] = { [41] = '\1' }; // make the last character non-zero so we can test based on it later
    // rand_str(str, sizeof str - 1);
    // assert(str[41] == '\0');      // test the correct insertion of string terminator
    // puts(str);

    // rand_str(str, sizeof str - 1);
    // assert(str[41] == '\0');      // test the correct insertion of string terminator
    // puts(str);

	// char *args[4];

	// args[0] = (char *) "localhost:7390";
	// args[1] = (char *) "localhost:7391";
	// args[2] = (char *) "localhost:7392";
	// args[3] = 0;

	// char buffer[2049];
 //        memset(buffer, 0, sizeof buffer);


	// if (kv739_init(args)==0) {
	// 	printf("kv739_init() OK\n");
	// }
	// else {
	// 	printf("kv739_init() Failed\n");
	// }


	// int ret = kv739_get((char *) "apple", buffer);

	// printf("value=%s, status=%d\n", buffer, ret);

	// ret = kv739_put((char *) "orange", (char *) "good", buffer);
	// printf("value=%s, status=%d\n", buffer, ret);

	// ret = kv739_get((char *) "orange", buffer);
	// printf("value=%s, status=%d\n", buffer, ret);

	printf("** calling kv739_shutdown()...\n");
	if (kv739_shutdown()==0) {
		printf("** kv739_shutdown() OK\n");
	}
	else {
		printf("** kv739_shutdown() Failed!\n");
	}

	return 0;
}
