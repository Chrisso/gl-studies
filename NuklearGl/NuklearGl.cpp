// NuklearGl.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "NuklearGl.h"
#include "GlMainWindow.h"

CAppModule _Module;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	ATLTRACE(_T("Initializing application...\n"));
	ATLENSURE_SUCCEEDED(::CoInitialize(NULL));
	::AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES);
	_Module.Init(NULL, hInstance);

	CGlMainWindow win;
	if (!win.CreateEx(NULL, NULL))
	{
		::AtlMessageBox(NULL, _T("Failed to create main window! Application will exit now."));
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
	::CoUninitialize();

	ATLTRACE(_T("Exiting application.\n"));
	return (int)msg.wParam;
}
