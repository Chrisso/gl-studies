#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

#define ZLIB_WINAPI // only with static link!
#include <zlib.h>
#include <minizip/unzip.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlwin.h>

#include <atlapp.h>			// base WTL classes
extern CAppModule _Module;	// WTL version of CComModule
#include <atlframe.h>		// WTL frame window classes
#include <atlcrack.h>		// WTL enhanced msg map macros
#include <atlctrls.h>		// WTL windows controls
