#include "pch.h"
#include "GlMainWindow.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CGlMainWindow::CGlMainWindow()
{
}

CGlMainWindow::~CGlMainWindow()
{
}

/////////////////////////////////////////////////////////////////
// Method Implementation/ Windows Message Handling
/////////////////////////////////////////////////////////////////

int CGlMainWindow::OnCreate(CREATESTRUCT *lpcs)
{
	ATLTRACE(_T("OnCreate\n"));

	// create command bar menu
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.AttachMenu(GetMenu()); // attach menu
	SetMenu(NULL);					// remove old menu

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);

	// create status bar
	CreateSimpleStatusBar();
	int nPanes[] = { ID_DEFAULT_PANE, ID_INFO_PANE };
	m_StatusBar.SubclassWindow(m_hWndStatusBar);
	m_StatusBar.SetPanes(nPanes, sizeof(nPanes) / sizeof(int), false);
	m_StatusBar.SetPaneText(ID_DEFAULT_PANE, _T("Ready"));

	if (!InitGlew())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	m_hWndClient = m_View.Create(
		m_hWnd, rcDefault, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_CLIENTEDGE
	);

	if (!m_hWndClient)
	{
		// no sense to run without a client
		return -1;
	}

	if (!m_FrameCounter.Create())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	m_FrameCounter.Subscribe([this](size_t fps) {
		::PostMessage(this->m_hWnd, WM_APP_BENCHMARK, 0, (LPARAM)fps);
	});

	USES_CONVERSION;
	m_StatusBar.SetPaneText(
		ID_DEFAULT_PANE,
		(LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
	);

	UpdateLayout();
	SetMsgHandled(FALSE);

	return 0;
}

int CGlMainWindow::OnClose()
{
	ATLTRACE(_T("OnClose\n"));
	DestroyWindow();
	return 0;
}

int CGlMainWindow::OnDestroy()
{
	ATLTRACE(_T("OnDestroy\n"));
	PostQuitMessage(0);
	return 0;
}

LRESULT CGlMainWindow::OnBenchmark(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CString fmt;
	fmt.Format(_T("%d FPS"), lParam);
	m_StatusBar.SetPaneText(ID_INFO_PANE, fmt);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Main-Menu message handling
///////////////////////////////////////////////////////////////////////////////

void CGlMainWindow::OnFileExit(UINT uNotifyCode, int nID, HWND hWnd)
{
	PostMessage(WM_CLOSE);
}

void CGlMainWindow::OnHelpInfo(UINT uNotifyCode, int nID, HWND hWnd)
{
	::AtlMessageBox(m_hWnd, IDS_APP_INFO, IDR_MAINFRAME);
}

///////////////////////////////////////////////////////////////////////////////
// Application logic
///////////////////////////////////////////////////////////////////////////////

#define OGL_HELPER_WINDOW _T("CS_WINDOW-Helper")

bool CGlMainWindow::InitGlew()
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = ::DefWindowProc;
	wcex.hInstance = _Module.GetModuleInstance();
	wcex.lpszClassName = OGL_HELPER_WINDOW;
	::RegisterClassEx(&wcex);

	HWND hWnd = ::CreateWindow(
		OGL_HELPER_WINDOW, _T("init"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, _Module.GetModuleInstance(), NULL);

	bool bResult = false;
	if (hWnd)
	{
		HDC hDC = ::GetDC(hWnd);

		if (hDC)
		{
			PIXELFORMATDESCRIPTOR pfd;
			ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = ::GetDeviceCaps(hDC, BITSPIXEL);

			int iPixelFormat = ::ChoosePixelFormat(hDC, &pfd);
			if (iPixelFormat && ::SetPixelFormat(hDC, iPixelFormat, &pfd))
			{
				USES_CONVERSION;
				HGLRC hRC = ::wglCreateContext(hDC);
				::wglMakeCurrent(hDC, hRC);

				GLenum err = glewInit();
				if (err == GLEW_OK)
				{
					ATLTRACE(_T("GL_VENDOR:   %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
					ATLTRACE(_T("GL_RENDERER: %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
					ATLTRACE(_T("GL_VERSION:  %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
					ATLTRACE(_T("GL_SLVSN:    %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
					bResult = true;
				}
				else
				{
					CString msg(CA2CT(reinterpret_cast<const char*>(glewGetErrorString(err))));
					ATLTRACE(_T("initGlew: %s\n"), (LPCTSTR)msg);
				}

				::wglMakeCurrent(NULL, NULL);
				::wglDeleteContext(hRC);
			}

			::ReleaseDC(hWnd, hDC);
		}
	}

	::DestroyWindow(hWnd);
	::UnregisterClass(OGL_HELPER_WINDOW, _Module.GetModuleInstance());

	return bResult;
}

void CGlMainWindow::Render()
{
	m_View.Render(m_FrameCounter.Tick() / 10.0f);
}
