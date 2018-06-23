#pragma once

// requires:
// #include <atlstr.h>
// #include <shlwapi.h>
// #include <shlobj.h>

#ifndef APP_VENDOR
	#define APP_VENDOR _T("cstoepel")
#endif

CString GetAppFileName(LPCTSTR szFile, bool bCreatePath = true)
{
	TCHAR szModule[MAX_PATH];
	::GetModuleFileName(NULL, szModule, MAX_PATH);

	TCHAR *szExecutable = ::PathFindFileName(szModule);
	::PathRemoveExtension(szExecutable);

	TCHAR szAppFile[MAX_PATH];
	::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szAppFile);

	::PathAppend(szAppFile, APP_VENDOR);
	::PathAppend(szAppFile, szExecutable);

	if (bCreatePath && !::PathFileExists(szAppFile))
		::SHCreateDirectory(NULL, szAppFile);

	::PathAppend(szAppFile, szFile);

	return CString(szAppFile);
}

#ifdef _DEBUG
	#define APP_FILE(X) _T(X)
	#define APP_FILE_UNCHECKED(X) _T(X)
	#define APP_FILE_EXISTS(X) ::PathFileExists(_T(X))
#else
	#define APP_FILE(X) ((LPCTSTR)GetAppFileName(_T(X), true))
	#define APP_FILE_UNCHECKED(X) ((LPCTSTR)GetAppFileName(_T(X), false))
	#define APP_FILE_EXISTS(X) ::PathFileExists(GetAppFileName(_T(X), false))
#endif
