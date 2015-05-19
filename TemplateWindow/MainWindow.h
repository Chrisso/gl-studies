#pragma once

class CMainWindow : public CFrameWindowImpl<CMainWindow>,
	public CUpdateUI<CMainWindow>,
	public CMessageFilter,
	public CIdleHandler
{
private:
	CCommandBarCtrl m_CmdBar;

public:
	CMainWindow();
	~CMainWindow();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	LRESULT OnCreate(CREATESTRUCT *lpcs);
	LRESULT OnClose();
	LRESULT OnDestroy();
	LRESULT OnSize(UINT uParam, const CSize& size);

	void OnFileExit(UINT wParam, int lParam, HWND hWnd);
	void OnHelpInfo(UINT wParam, int lParam, HWND hWnd);

	DECLARE_FRAME_WND_CLASS_EX(_T("CS_WINDOW-Template"), IDR_MAINFRAME, CS_DBLCLKS, COLOR_WINDOW);

	BEGIN_MSG_MAP(CGlMainWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)

		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnHelpInfo)

		CHAIN_MSG_MAP(CUpdateUI<CMainWindow>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainWindow>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainWindow)
		// TODO
	END_UPDATE_UI_MAP()
};

