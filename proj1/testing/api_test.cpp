#include <iostream>
#include <memory>
#include <vector>
#include <memory.h>
#include "lib739kv.h"


int main() {

	char *args[4];

	args[0] = (char *) "localhost:7390";
	args[1] = (char *) "localhost:7391";
	args[2] = (char *) "localhost:7392";
	args[3] = 0;

	char buffer[2049];
        memset(buffer, 0, sizeof buffer);


	if (kv739_init(args)==0) {
		printf("kv739_init() OK\n");
	}
	else {
		printf("kv739_init() Failed\n");
	}


	int ret = kv739_get((char *) "apple", buffer);

	printf("value=%s, status=%d\n", buffer, ret);

	ret = kv739_put((char *) "orange", (char *) "good", buffer);
	printf("value=%s, status=%d\n", buffer, ret);

	ret = kv739_get((char *) "orange", buffer);
	printf("value=%s, status=%d\n", buffer, ret);

	if (kv739_shutdown()==0) {
		printf("kv739_shutdown() OK\n");
	}
	else {
		printf("kv739_shutdown() Failed!\n");
	}

	return 0;
}
