#include "stdafx.h"
#include <shlwapi.h>
#include <shlobj.h>
#include "AppFile.h"

CString GetAppFileName(LPCTSTR szFile, bool bCreatePath)
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
