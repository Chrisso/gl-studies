#include "stdafx.h"
#include "GlMainWindow.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
////////////////////////////////////////////////////////////////

CGlMainWindow::CGlMainWindow() : m_pScene(NULL), m_hDC(NULL), m_hRC(NULL)
{
}

CGlMainWindow::~CGlMainWindow()
{
}

/////////////////////////////////////////////////////////////////
// Method Implemenatation/ Windows Messge Handling
/////////////////////////////////////////////////////////////////

LRESULT CGlMainWindow::OnCreate(CREATESTRUCT *lpcs)
{
	LOGMSG_DEBUG(_T("OnCreate\n"));

	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);// create command bar window
	m_CmdBar.AttachMenu(GetMenu());			// attach menu
	m_CmdBar.LoadImages(IDR_MAINFRAME);		// load command bar images
	SetMenu(NULL);							// remove old menu

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	
	int nPanes[] = { ID_DEFAULT_PANE, IDS_FPS_PANE };
	m_hWndStatusBar = m_StatusBar.Create(m_hWnd);
	m_StatusBar.SetPanes(nPanes, 2, false);
	m_StatusBar.SetPaneText(ID_DEFAULT_PANE, CString((LPCTSTR)IDS_READY));

	if (!InitGlew())
	{
		LOGMSG_ERROR(_T("Could initialize graphics extensions!\n"));
		return -1;
	}

	m_hDC = GetDC();
	if (!m_hDC)
	{
		LOGMSG_ERROR(_T("Could not get device context!\n"));
		return -1;
	}

	int PixelFormat = 0;
	PIXELFORMATDESCRIPTOR pfd;

	if (WGLEW_ARB_pixel_format && WGLEW_ARB_multisample)
	{
		UINT  nFormats = 0;
		int   nPixelFormat  = 0;
		float fAttributes[] = { 0.0f, 0.0f };
		int   nAttributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 16,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 8,
			0, 0 };

		wglChoosePixelFormatARB(m_hDC, nAttributes, fAttributes, 1, &nPixelFormat, &nFormats);
		if (nFormats)
		{
			LOGMSG_DEBUG(_T("Multisampling is available!\n"));
			PixelFormat = nPixelFormat;
		}
	}
	
	if (!PixelFormat)
	{
		LOGMSG_DEBUG(_T("Proceeding without multisampling.\n"));
		ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = GetDeviceCaps(m_hDC, BITSPIXEL);
		pfd.cDepthBits = 16;
		PixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	}

	if (!PixelFormat)
	{
		LOGMSG_ERROR(_T("Could not choose pixelformat!\n"));
		return -1;
	}
	if (!SetPixelFormat(m_hDC, PixelFormat, &pfd))
	{
		LOGMSG_ERROR(_T("Could not set pixelformat!\n"));
		return -1;
	}

	m_hRC = wglCreateContext(m_hDC);
	if (!m_hRC)
	{
		LOGMSG_ERROR(_T("Could not create OpenGL context!\n"));
		return -1;
	}

	if (!wglMakeCurrent(m_hDC, m_hRC))
	{
		LOGMSG_ERROR(_T("Could not set OpenGL context!\n"));
		return -1;
	}

	m_pScene = new CScene();
	m_Timer.SetBenchmarkTarget(m_hWnd);

#ifdef _DEBUG
	USES_CONVERSION;
	LOGMSG_DEBUG(Logging::CLogMessage(_T("%s\n"), CA2CT(reinterpret_cast<const char*>(glGetString(GL_VENDOR)))));
	LOGMSG_DEBUG(Logging::CLogMessage(_T("%s\n"), CA2CT(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))));
	LOGMSG_DEBUG(Logging::CLogMessage(_T("%s\n"), CA2CT(reinterpret_cast<const char*>(glGetString(GL_VERSION)))));
#endif

	UpdateLayout();
	SetMsgHandled(FALSE);

	return 0;
}

LRESULT CGlMainWindow::OnClose()
{
	LOGMSG_DEBUG(_T("OnClose\n"));
	DestroyWindow();
	return 0;
}

LRESULT CGlMainWindow::OnDestroy()
{
	LOGMSG_DEBUG(_T("OnDestroy\n"));
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_hRC);
	ReleaseDC(m_hDC);
	PostQuitMessage(0);
	return 0;
}

void CGlMainWindow::Render()
{
	if (m_pScene)
	{
		m_pScene->Render(m_Timer.Tick());
		SwapBuffers(m_hDC);
	}
}

LRESULT CGlMainWindow::OnSize(UINT uParam, const CSize& size)
{
	if (m_pScene)
		m_pScene->Resize(size.cx, size.cy);

	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CGlMainWindow::OnBenchmark(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case BENCHMARK_FPS:
		{
			CString formatter;
			formatter.Format(_T("%d FPS"), lParam);
			m_StatusBar.SetPaneText(IDS_FPS_PANE, formatter);
		} break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Main-Menu message handling
///////////////////////////////////////////////////////////////////////////////

void CGlMainWindow::OnFileExit(UINT wParam, int lParam, HWND hWnd)
{
	PostMessage(WM_CLOSE);
}

void CGlMainWindow::OnHelpInfo(UINT wParam, int lParam, HWND hWnd)
{
	::AtlMessageBox(m_hWnd, _T("Info message"), IDR_MAINFRAME);
}

///////////////////////////////////////////////////////////////////////////////
// Internal method implementation
///////////////////////////////////////////////////////////////////////////////

#define OGL_HELPER_WINDOW _T("CS_WINDOW-Helper")

BOOL CGlMainWindow::InitGlew()
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = ::DefWindowProc;
	wcex.hInstance = _Module.GetModuleInstance();
	wcex.lpszClassName = OGL_HELPER_WINDOW;
	::RegisterClassEx(&wcex);

	HWND hWnd = ::CreateWindow(
		OGL_HELPER_WINDOW, _T("init"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, _Module.GetModuleInstance(), NULL);

	BOOL bResult = FALSE;
	if (hWnd)
	{
		HDC hDC = ::GetDC(hWnd);

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = GetDeviceCaps(m_hDC, BITSPIXEL);
		pfd.cDepthBits = 16;

		int iPixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (iPixelFormat && SetPixelFormat(hDC, iPixelFormat, &pfd))
		{
			HGLRC hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			bResult = (glewInit() == GLEW_OK)? TRUE : FALSE;

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
		}

		::ReleaseDC(hWnd, hDC);
	}

	::DestroyWindow(hWnd);

	::UnregisterClass(OGL_HELPER_WINDOW, _Module.GetModuleInstance());

	return bResult;
}
