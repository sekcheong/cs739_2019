#include <iostream>
#include <memory>
#include <vector>
#include "lib739kv.h"


int main() {
	
	char *args[4];

	args[0] = (char *) "coco:500100";
	args[1] = (char *) "mango:500200";
	args[2] = (char *) "banana:500300";
	args[3] = 0;

	char buffer[2048];

	kv739_init(args);
	kv739_get((char *) "apple", buffer);
	kv739_put((char *) "orange", (char *) "good", buffer);
	kv739_shutdown();

	return 0;
}