#include <iostream>
#include <memory>
#include <vector>
#include <memory.h>
#include "lib739kv.h"


// void args_pack(char** list, char *buff) {
// 	int i=0;
// 	char *p = buff;
// 	while (list[i]!=0) {
// 		int len = strlen(list[i]);
// 		memcpy(p, list[i], len);
// 		p[len] = '|';
// 		p += (len+1);
// 		i++;
// 	}
// 	p[-1]=0;
// }


// void args_unpack(char **list, char *buff) {
// 	int i = 0;
// 	int j = 0;
// 	char *p = buff;
// 	int len = strlen(buff);
// 	list[j] = (char *) p;
// 	j = j + 1;
// 	for (int i=0; i<len-1; i++) {
// 		if (buff[i]=='|') {
// 			list[j] = (char *) &buff[i+1];
// 			buff[i] = 0;
// 			j++;
// 		}
// 	}
// 	list[j] = 0;
// }


// void args_print(char **list) {
// 	int i=0;
// 	while (list[i]!=0) {
// 		printf("%d %s\n", i, list[i]);
// 		i++;
// 	}
// }


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

	// char *l[5];
	// args_pack(args, buffer);
	// args_unpack(l, buffer);
	// args_print(l);

	return 0;
}