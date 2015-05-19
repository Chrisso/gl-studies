// TemplateWindow.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TemplateWindow.h"
#include "MainWindow.h"

CAppModule _Module;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	ATLTRACE(_T("[WinMain] Starting application...\n"));

	Logging::CLoggerFactory::getDefaultInstance()->AddTarget(new Logging::CLogTargetDebugger(Logging::LOG_LEVEL_DEBUG));
	Logging::CLoggerFactory::getDefaultInstance()->AddTarget(new Logging::CLogTargetMessageBox(Logging::LOG_LEVEL_ERROR));
	LOGMSG_INFO(_T("Starting application and initializing diagnostics...\n"));

	::CoInitialize(NULL);
	::AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES);
	_Module.Init(NULL, hInstance);

	CMessageLoop loop;
	_Module.AddMessageLoop(&loop);

	CMainWindow win;
	if (!win.CreateEx(NULL, CRect(100, 100, 800, 600)))
	{
		LOGMSG_ERROR(_T("Failed to create main window! Application will exit now.\n"));
		return 1;       // Window creation failed
	}

	win.ShowWindow(nCmdShow);
	win.UpdateWindow();

	int res = loop.Run();

	_Module.Term();
	::CoUninitialize();

	ATLTRACE(_T("[WinMain] Exiting application\n"));
	return res;
}
