#include <iostream>
#include <string>

#include "dsserver.h"

int main() {
	std::cout << "Enter 'q' to stop...";
	std::string host = "localhost";
	std::string dbfile = "test.db";
	dsserver s(host, 52123, dbfile);
	s.serve();

	while (1) {
		std::string ans;
		std::cin >> ans;
		if (ans=="q") break;
	}
}