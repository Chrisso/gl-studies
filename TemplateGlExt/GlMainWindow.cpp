#include "stdafx.h"
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

	m_hDC = GetDC();
	if (!m_hDC)
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	if (!InitGlew())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	UINT nFormats = 0;
	int nPixelFormat = 0;
	int nAttributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, ::GetDeviceCaps(m_hDC, BITSPIXEL),
		WGL_DEPTH_BITS_ARB, 16,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 8,
		0
	};

	if (!wglChoosePixelFormatARB(m_hDC, nAttributes, NULL, 1, &nPixelFormat, &nFormats) || !nFormats)
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	PIXELFORMATDESCRIPTOR pfd;

	if (!nPixelFormat ||
		!::DescribePixelFormat(m_hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) ||
		!::SetPixelFormat(m_hDC, nPixelFormat, &pfd))
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	int nContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	m_hRC = wglCreateContextAttribsARB(m_hDC, NULL, nContextAttributes);
	if (!m_hRC || !wglMakeCurrent(m_hDC, m_hRC))
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	m_pScene = new CScene();
	if (!m_pScene->Create())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	m_FrameCounter.SetTargetWindow(m_hWnd);

	USES_CONVERSION;
	ATLTRACE(_T("GL_VENDOR:   %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
	ATLTRACE(_T("GL_RENDERER: %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
	ATLTRACE(_T("GL_VERSION:  %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
	ATLTRACE(_T("GL_SLVSN:    %s\n"), (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
	m_StatusBar.SetPaneText(ID_DEFAULT_PANE, (LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));

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

	if (m_pScene)
	{
		delete m_pScene;
		m_pScene = NULL;
	}

	if (m_hRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hRC);
		m_hRC = NULL;
	}

	if (m_hDC)
	{
		ReleaseDC(m_hDC);
		m_hDC = NULL;
	}

	PostQuitMessage(0);
	return 0;
}

int CGlMainWindow::OnSize(UINT nType, CSize size)
{
	if (m_pScene)
	{
		glViewport(0, 0, size.cx, size.cy);
		m_pScene->Resize(size.cx, size.cy);
	}

	SetMsgHandled(FALSE);
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
			pfd.cColorBits = ::GetDeviceCaps(m_hDC, BITSPIXEL);

			int iPixelFormat = ::ChoosePixelFormat(hDC, &pfd);
			if (iPixelFormat && ::SetPixelFormat(hDC, iPixelFormat, &pfd))
			{
				HGLRC hRC = ::wglCreateContext(hDC);
				::wglMakeCurrent(hDC, hRC);

				GLenum err = glewInit();
				if (err != GLEW_OK)
				{
					USES_CONVERSION;
					CString msg(CA2CT(reinterpret_cast<const char*>(glewGetErrorString(err))));
					ATLTRACE(_T("initGlew: %s\n"), (LPCTSTR)msg);
				}
				else bResult = true;

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
	if (!m_hDC || !m_hRC) return;

	if (m_pScene)
	{
		m_pScene->Render(m_FrameCounter.Tick() / 10.0f);
	}

	::SwapBuffers(m_hDC);
}
