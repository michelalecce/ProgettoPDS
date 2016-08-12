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
}

AppInfo::AppInfo(HWND wnd):wnd(wnd){
	HANDLE hproc;
	GetWindowThreadProcessId(wnd,&pid); //we recover the pid of the process linked to the window
	if((hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid))==NULL){
		DWORD err = GetLastError();
		throw WindowInfoException(err, "Error while opening the handle to a process");
	}
	namesize = GetModuleFileNameEx(hproc, NULL, name, MAX_PATH - 1); //retrieve the name of the process, we need the handle to do this
	if(namesize==0){
		throw WindowInfoException(GetLastError(), "Problems with process name");
	}
	else{
		name[namesize] = (TCHAR)'\0';
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

AppInfo::~AppInfo(){
}

void AppInfo::retrieveIcon(){ //taken principally from STACKOVERFLOW
	SHFILEINFO info;
	if(SHGetFileInfo(name, 0, &info, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON)==0){
		int err = GetLastError();
		std::cout << "Error while retrieving info for icon errno= " << err << "\tpid:" << pid << std::endl;
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
		std::cout << "Error retrieving icon for pid:" << std::to_string(pid) << std::endl;
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
		std::cout << "Error retrieving icon for pid:" << std::to_string(pid) << std::endl;
		iconFileSize = 0;
		return;
	}
	else{
		// Write the stream content to the file
		HGLOBAL hBuf = 0;
		GetHGlobalFromStream(pStream, &hBuf);
		void* buffer = GlobalLock(hBuf);
		_stprintf_s(iconFile, MAX_PATH,TEXT("%d.ico"), pid);
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

void AppInfo::cleanIcon(){
	DeleteFile(iconFile);
}