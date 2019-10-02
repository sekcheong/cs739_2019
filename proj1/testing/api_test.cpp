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

	char buffer[2048];
        int i = 0;

	kv739_init(args);

	kv739_get((char *) "apple", buffer);

	printf("value=%s, status=%d\n", buffer, i);

	kv739_put((char *) "orange", (char *) "good", buffer);
	printf("value=%s, status=%d\n", buffer, i);

	kv739_get((char *) "orange", buffer);
	printf("value=%s, status=%d\n", buffer, i);

	kv739_shutdown();

	// char *l[5];
	// args_pack(args, buffer);
	// args_unpack(l, buffer);
	// args_print(l);

	return 0;
}
