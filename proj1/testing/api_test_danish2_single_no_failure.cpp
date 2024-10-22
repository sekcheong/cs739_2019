#include <iostream>
#include <memory>
#include <vector>
#include <memory.h>
#include "lib739kv.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define _BSD_SOURCE

#include <sys/time.h>
#include <assert.h>
#define SERVER_LOC_SIZE 50
#define MAX_VAL_SIZE 2048

#ifndef KVSTORE_SIZE
    #define KVSTORE_SIZE 100 // In terms of number of keys
#endif

#ifndef SLEEP_TIME_US
    #define SLEEP_TIME_US 0 // Sleep time in microseconds
#endif

char *rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

float get_time_elapsed_sec(struct timeval tv1, struct timeval tv2) {
    struct timeval tvdiff = { tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec };
    if (tvdiff.tv_usec < 0) { tvdiff.tv_usec += 1000000; tvdiff.tv_sec -= 1; }
    return tvdiff.tv_sec + tvdiff.tv_usec/(1000.0*1000.0);
}

int main(int argc, char *argv[])
{
    //Py_Initialize();
    //initclient();
   
    int num_servers = 3; 
    // Test code starts here
    char **servers;
    servers = (char**)malloc((num_servers+1)*sizeof(char*));

    for(int i=0;i<=num_servers;i++) {
	servers[i] = (char*)malloc(SERVER_LOC_SIZE*sizeof(char));
    }
    strcpy(servers[0], "localhost:8003");
    strcpy(servers[1], "localhost:8004");
    strcpy(servers[2], "localhost:8005");
    servers[num_servers] = 0;

    int ret = kv739_init(servers);
    printf("Arjun Kv_init %d\n", ret);

    if(ret == -1){
	    exit(0);
    }
    
    // Now generate KVSTORE_SIZE random keys
    char **keys;
    keys = (char**) malloc(KVSTORE_SIZE*sizeof(char*));
    for(int i=0;i<KVSTORE_SIZE;i++) {
        keys[i] = (char*)malloc(MAX_VAL_SIZE*sizeof(char));
	rand_string(keys[i], 128);
    }

    // Put seed values for 100000 random keys
    char **values;
    values = (char**) malloc(KVSTORE_SIZE*sizeof(char*));
    for(int i=0;i<KVSTORE_SIZE;i++) {
	values[i] = (char*)malloc(MAX_VAL_SIZE*sizeof(char));
        rand_string(values[i], 512);
    }

    // Insert the KV pairs
    char *old_val;
    old_val = (char*) malloc(MAX_VAL_SIZE * sizeof(char));
    for(int i=0;i<KVSTORE_SIZE;i++) {
       ret = kv739_put(keys[i], values[i], old_val);
       //assert(ret == 1); // there should be no failure
       //assert(old_val[0] == '\0'); // old value should be NULL
    }

    // Allow for eventual consistency to play out its magic
    usleep(SLEEP_TIME_US);

    // Check if we are able to read back same values
    struct timeval tv1, tv2;
    int errors = 0;
    int num_iterations = 5;
    float tot = 0;
    while(num_iterations > 0) {
       errors = 0;
       gettimeofday(&tv1, NULL);
       for(int i=0;i<KVSTORE_SIZE;i++) {
           ret = kv739_get(keys[i], old_val);
	   if (ret != 0) {
               printf("Arjun Read ret: %d\n", ret);
	   }
           //assert(ret == 0); // there should be no failure
           if(strcmp(old_val, values[i]) != 0) {
               errors++;
           }
       }
       gettimeofday(&tv2, NULL);
       num_iterations--;
       printf("Arjun Error in reading back same value: %f percent\n", (errors*100.0)/KVSTORE_SIZE);
       printf("Arjun Read throughput = %f keys/sec\n", (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2));
       tot = tot + (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2);
    }
    printf("Arjun Avg. Read throughput = %f keys/sec\n", tot/10);

    // Now test atomic put and get
    // Check if we are able to read back same values
    num_iterations = 5;
    tot = 0;
    while(num_iterations > 0) {
    	errors = 0;
    	char *val;
    	val = (char*) malloc(MAX_VAL_SIZE * sizeof(char));
	gettimeofday(&tv1, NULL);
    	for(int i=0;i<KVSTORE_SIZE;i++) {
       	    rand_string(val, 512);
            ret = kv739_put(keys[i], val, old_val);
	    if (ret != 0) {
               printf("Arjun write ret: %d\n", ret);
            }
            //assert(ret == 0); // there should be no failure
            if(strcmp(old_val, values[i]) != 0) {
	       //printf("old: %s\nnew: %s\n", old_val, values[i]);
               errors++;
            }
            strcpy(values[i], val);
        }
	gettimeofday(&tv2, NULL);
	num_iterations--;
        printf("Arjun Errors in putting value for same key: %f percent\n", (errors*100.0)/KVSTORE_SIZE);
	printf("Arjun Write throughput = %f keys/sec\n", (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2));
	tot = tot + (KVSTORE_SIZE * 1.0)/get_time_elapsed_sec(tv1,tv2);
	usleep(SLEEP_TIME_US);
    }
    printf("Arjun Avg. Write throughput = %f\n keys/sec", tot/10);
    //Py_Finalize();
    return 0;
}
