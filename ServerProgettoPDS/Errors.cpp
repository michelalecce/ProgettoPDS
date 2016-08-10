#pragma once
#include "stdafx.h"
#include <windows.h>
#include <stdexcept>

class WindowInfoException: public std::runtime_error {
private:
	DWORD error;
	HANDLE hproc;
public:
	WindowInfoException(DWORD err, std::string msg): std::runtime_error(msg), error(err){}
	DWORD getErr(void){ return error; }
};

class ListException: public std::runtime_error {
private:
	DWORD error;
public:
	ListException(DWORD err, std::string msg) : std::runtime_error(msg), error(err) {}
	DWORD getErr(void) { return error; }
};