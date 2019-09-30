#include <iostream>
#include <string>

#include "dsclient.h"

int main(int argc, char *argv[]) {
	
	if (argc<3) {
		std::cout << "Usage: " << std::endl;
		std::cout << "  test_client [host] [port]" << std::endl;
		return -1;
	}

	std::string host(argv[1]);
	int port = atoi(argv[2]);

	dsclient c(host, port);

	std::cout << "Host: " << host << std::endl;
	std::cout <<" Port: " << port << std::endl;

	std::cout << "Enter key,value pair, enter 'q' for key to shutdown the server." << std::endl;

	while (1) {
		
		std::string key;
		std::string value;

		std::cout << "Enter a key:";		
		std::cin >> key;

		if (key=="q") {
			message m(command::SHUT_DOWN);
			c.send_message(m, m);
			break;
		}
		
		std::cout << "Enter a value:";
		std::cin >> value;

		message msg(command::PUT);

		msg.set_key(key.c_str());
		msg.set_value(value.c_str(), value.length());

		message res;
		c.send_message(msg, res);

	}
}