// TemplateGlExt.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TemplateGlExt.h"
#include "GlMainWindow.h"

CAppModule _Module;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	CoInitialize(NULL);
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES);
	_Module.Init(NULL, hInstance);

	Logging::CLoggerFactory::getDefaultInstance()->AddTarget(new Logging::CLogTargetDebugger(Logging::LOG_LEVEL_DEBUG));
	Logging::CLoggerFactory::getDefaultInstance()->AddTarget(new Logging::CLogTargetMessageBox(Logging::LOG_LEVEL_ERROR));
	LOGMSG_INFO(_T("Starting application and initializing diagnostics...\n"));

	CGlMainWindow win;
	if (!win.CreateEx(NULL, CRect(100, 100, 800, 600)))
	{
		LOGMSG_ERROR(_T("Failed to create main window! Application will exit now.\n"));
		return 1; // Window creation failed
	}

	win.ShowWindow(nCmdShow);
	win.UpdateWindow();

	// Windows Message Loop
	MSG msg;
	bool bDone = false;
	while (!bDone)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			win.Render();
		}
	}

	_Module.Term();
	CoUninitialize();

	LOGMSG_DEBUG(_T("Exiting application.\n"));
	return (int)msg.wParam;
}
