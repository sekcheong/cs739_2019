#ifndef SQLRESULT_H
#define SQLRESULT_H

#include "sqlstatement.h"

class sql_result {
public:
	sql_result(std::shared_ptr<sql_statement> stmt) {
		_stmt_p = stmt;
	}
	virtual ~sql_result() {
		_stmt_p = nullptr;
	}
	
	bool read() {
		if (!_stmt_p) return false;
		return _stmt_p->read();
	}

	std::string read_text(int col) {
		return _stmt_p->read_text(col);
	}

	int read_blob(int col, char *buffer, int length) {
		return _stmt_p->read_blob(col, buffer, length);
	}
	int64_t read_int64(int col) {
		return _stmt_p->read_int64(col);
	}
private:
	std::shared_ptr<sql_statement> _stmt_p = nullptr;
};
#endif