#pragma once

// requires:
// #include <atlstr.h>

#ifndef APP_VENDOR
	#define APP_VENDOR _T("cstoepel")
#endif

CString GetAppFileName(LPCTSTR szFile, bool bCreatePath = true);

#ifdef _DEBUG
	#define APP_FILE(X) _T(X)
	#define APP_FILE_UNCHECKED(X) _T(X)
	#define APP_FILE_EXISTS(X) ::PathFileExists(_T(X))
#else
	#define APP_FILE(X) ((LPCTSTR)GetAppFileName(_T(X), true))
	#define APP_FILE_UNCHECKED(X) ((LPCTSTR)GetAppFileName(_T(X), false))
	#define APP_FILE_EXISTS(X) ::PathFileExists(GetAppFileName(_T(X), false))
#endif
