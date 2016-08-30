#pragma once
#include "stdafx.h"
#include "windows.h"
#include <stdexcept>

class WindowInfoException: public std::runtime_error {
private:
	DWORD error;
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

class IconSendException: public std::runtime_error{
private:
	DWORD error;
public:	
	IconSendException(DWORD err,std::string msg):std::runtime_error(msg), error(err) {}
	DWORD getErr(void) { return error; }
};

class FocusSendException : public std::runtime_error {
private:
	DWORD error;
public:
	FocusSendException(DWORD err, std::string msg) :std::runtime_error(msg), error(err) {}
	DWORD getErr(void) { return error; }
};

class ReadException : public std::runtime_error {
private:
	DWORD error;
public:
	ReadException(DWORD err, std::string msg) :std::runtime_error(msg), error(err) {}
	DWORD getErr(void) { return error; }
};

class CommandException : public std::runtime_error {
private:
	DWORD error;
public:
	CommandException(DWORD err, std::string msg) : std::runtime_error(msg), error(err) {}
	DWORD getErr(void) { return error; }
};