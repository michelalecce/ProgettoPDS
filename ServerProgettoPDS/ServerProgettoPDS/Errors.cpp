#pragma once
#include <stdexcept>
#include <windows.h>

class WindowInfoException : std::exception {
protected:
	DWORD err;
public:
	WindowInfoException(DWORD err) :err(err) {
	}
	DWORD getErrCode() { return err; }
};

class ListException : std::exception {
protected:
	DWORD err;
public:
	ListException(DWORD err) :err(err) {
	}
	DWORD getErrCode() { return err; }
};