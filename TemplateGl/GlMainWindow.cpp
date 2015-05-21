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

	m_hDC = GetDC();
	if (!m_hDC)
	{
		LOGMSG_ERROR(_T("Could not get device context!\n"));
		return -1;
	}

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = GetDeviceCaps(m_hDC, BITSPIXEL);
	pfd.cDepthBits = 16;

	int PixelFormat = ChoosePixelFormat(m_hDC, &pfd);
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
