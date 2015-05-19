#include "stdafx.h"
#include "Resource.h"
#include "MainWindow.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
////////////////////////////////////////////////////////////////

CMainWindow::CMainWindow()
{
}

CMainWindow::~CMainWindow()
{
}

/////////////////////////////////////////////////////////////////
// Method Implementation/ Windows Message Handling
/////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnCreate(CREATESTRUCT *lpcs)
{
	LOGMSG_DEBUG(_T("OnCreate\n"));

	// create command bar menu
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.AttachMenu(GetMenu());			// attach menu
	SetMenu(NULL);							// remove old menu

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	CreateSimpleStatusBar();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	::SendMessage(m_hWndStatusBar, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)CString((LPCTSTR)IDS_READY));

	UIAddMenuBar(m_CmdBar);
	UpdateLayout();

	SetMsgHandled(FALSE);

	return 0;
}

LRESULT CMainWindow::OnClose()
{
	LOGMSG_DEBUG(_T("OnClose\n"));
	DestroyWindow();
	return 0;
}

LRESULT CMainWindow::OnDestroy()
{
	LOGMSG_DEBUG(_T("OnDestroy\n"));
	PostQuitMessage(0);
	return 0;
}

LRESULT CMainWindow::OnSize(UINT uParam, const CSize& size)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainWindow::PreTranslateMessage(MSG* pMsg)
{
	return CFrameWindowImpl<CMainWindow>::PreTranslateMessage(pMsg);
}

BOOL CMainWindow::OnIdle()
{
	UIUpdateMenuBar();
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Main-Menu message handling
///////////////////////////////////////////////////////////////////////////////

void CMainWindow::OnFileExit(UINT wParam, int lParam, HWND hWnd)
{
	PostMessage(WM_CLOSE);
}

void CMainWindow::OnHelpInfo(UINT wParam, int lParam, HWND hWnd)
{
	::AtlMessageBox(m_hWnd, _T("Info message."), IDR_MAINFRAME);
}
