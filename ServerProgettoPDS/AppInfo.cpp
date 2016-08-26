#include "stdafx.h"
#include "AppInfo.h"
#include "Errors.cpp"
#include "Psapi.h"
#include <iostream>
#include "Shellapi.h"
#include <olectl.h>
#pragma comment(lib, "oleaut32.lib")

AppInfo::AppInfo(){
	wnd=NULL;
	pid=0;
	namesize=0;
	still_open=false;
}

AppInfo::AppInfo(HWND wnd):wnd(wnd){
	HANDLE hproc;
	GetWindowThreadProcessId(wnd,&pid); //we recover the pid of the process linked to the window
	if((hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid))==NULL){
		std::cout << "Error while opening handle to a process"<< std::endl;
		_stprintf_s(name,MAX_PATH, _TEXT("Process name not retrievable"));
		sprintf_s(nameA,MAX_PATH ,"Process name not retrievable");
		namesize = strnlen(nameA, MAX_PATH);
		iconFileSize=0;
		return;
	}
	namesize = GetModuleFileNameEx(hproc, NULL, name, MAX_PATH - 1); //retrieve the name of the process, we need the handle to do this
	if(namesize==0){
		std::cout << "Error retrieving process name" << std::endl;
		_stprintf_s(name, MAX_PATH, _TEXT("Process name not retrievable"));
		sprintf_s(nameA, MAX_PATH, "Process name not retrievable");
		namesize = strnlen(nameA, MAX_PATH);
		iconFileSize = 0;
		CloseHandle(hproc);
		return;
	}
	//we do the same for the char version of the name, we need it for the socket... tchar was hard to receive on the client
	namesize = GetModuleFileNameExA(hproc, NULL, nameA, MAX_PATH - 1); //retrieve the name of the process, we need the handle to do this
	if (namesize == 0) {
		std::cout << "Error retrieving process nameA" << std::endl;
		_stprintf_s(name, MAX_PATH, _TEXT("Process name not retrievable"));
		sprintf_s(nameA, MAX_PATH, "Process name not retrievable");
		namesize = strnlen(nameA, MAX_PATH);
		iconFileSize = 0;
		CloseHandle(hproc);
		return;
	}
	//we retrieve the icon
	retrieveIcon();
	CloseHandle(hproc);
}

DWORD AppInfo::getPid()
{
	return pid;
}

DWORD AppInfo::getNameSize()
{
	return namesize;
}

TCHAR * AppInfo::getName()
{
	return (TCHAR *)name;
}

char * AppInfo::getNameA()
{
	return nameA;
}

HWND AppInfo::getWindow()
{
	return wnd;
}

AppInfo::~AppInfo(){
}
// versione tutte presenti ma brutte
void AppInfo::retrieveIcon(){ //taken principally from STACKOVERFLOW
	SHFILEINFO info;
	if(SHGetFileInfo(name, FILE_ATTRIBUTE_NORMAL, &info, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_LARGEICON)==0){
		int err = GetLastError();
		std::cout << "Error while retrieving info for icon errno= " << err << "\tpid:" << pid << "\t handle:"<< wnd << std::endl;
		char errbuf[ERRBUFF];
		strerror_s(errbuf, ERRBUFF, err);
		printf("%s\n", errbuf);
		iconFileSize=0;
	}
	// Create the IPicture intrface
	PICTDESC desc = { sizeof(PICTDESC) };
	desc.picType = PICTYPE_ICON;
	desc.icon.hicon = info.hIcon;
	IPicture* pPicture = 0;
	HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
	if (FAILED(hr)){
		std::cout << "Error retrieving icon for pid:" << pid << "\t handle:" << wnd << std::endl;
		iconFileSize = 0;
		return;
	}
	// Create a stream and save the image
	IStream* pStream = 0;
	CreateStreamOnHGlobal(0, TRUE, &pStream);
	LONG cbSize = 0;
	hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);

	
	if (FAILED(hr)) {
		pPicture->Release();
		std::cout << "Error retrieving icon for pid:" << pid << "\t handle:" << wnd << std::endl;
		iconFileSize = 0;
		return;
	}
	else{
		// Write the stream content to the file
		HGLOBAL hBuf = 0;
		GetHGlobalFromStream(pStream, &hBuf);
		void* buffer = GlobalLock(hBuf);
		_stprintf_s(iconFile, MAX_PATH,TEXT("%x.ico"), wnd);
		HANDLE hFile = CreateFile(iconFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
		else {
			DWORD written = 0;
			WriteFile(hFile, buffer, cbSize, &written, 0);
			iconFileSize= cbSize;
			CloseHandle(hFile);
		}
		GlobalUnlock(buffer);
	}
	// Cleanup
	pStream->Release();
	pPicture->Release();
}


/*
void AppInfo::retrieveIcon(){
	// Create the IPicture intrface
	HICON hIcon;
	hIcon = (HICON)SendMessage(wnd, WM_GETICON, ICON_SMALL2, 640);
	PICTDESC desc = { sizeof(PICTDESC) };
	desc.picType = PICTYPE_ICON;
	desc.icon.hicon = hIcon;
	IPicture* pPicture = 0;
	HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
	if (FAILED(hr)) {
		std::cout << "Error retrieving icon for pid:" << pid << "\t handle:" << wnd << std::endl;
		iconFileSize = 0;
		return;
	}
	// Create a stream and save the image
	IStream* pStream = 0;
	CreateStreamOnHGlobal(0, TRUE, &pStream);
	LONG cbSize = 0;
	hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);


	if (FAILED(hr)) {
		pPicture->Release();
		std::cout << "Error retrieving icon for pid:" << pid << "\t handle:" << wnd << std::endl;
		iconFileSize = 0;
		return;
	}
	else {
		// Write the stream content to the file
		HGLOBAL hBuf = 0;
		GetHGlobalFromStream(pStream, &hBuf);
		void* buffer = GlobalLock(hBuf);
		_stprintf_s(iconFile, MAX_PATH, TEXT("%x.ico"), wnd);
		HANDLE hFile = CreateFile(iconFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
		else {
			DWORD written = 0;
			WriteFile(hFile, buffer, cbSize, &written, 0);
			iconFileSize = cbSize;
			CloseHandle(hFile);
		}
		GlobalUnlock(buffer);
	}
	// Cleanup
	pStream->Release();
	pPicture->Release();
}
*/

void AppInfo::deleteIcon(){
	DeleteFile(iconFile);
}

TCHAR * AppInfo::getIconFile()
{
	return iconFile;
}

LONG AppInfo::getIconFileSize()
{
	return iconFileSize;
}

void AppInfo::setStillOpen()
{
	still_open = true;
}

void AppInfo::clearStillOpen()
{
	still_open= false;
}

bool AppInfo::getStillOpen()
{
	return still_open;
}
