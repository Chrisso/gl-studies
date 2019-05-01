#pragma once

#include <atlctrlw.h>
#include <atlctrlx.h>
#include <FrameCounter.h>

#include "Resource.h"
#include "GlView.h"

#define WM_APP_BENCHMARK (WM_APP+10)

class CGlMainWindow : public CFrameWindowImpl<CGlMainWindow>
{
private:
	CGlFrameCounter m_FrameCounter;
	CCommandBarCtrl m_CmdBar;
	CMultiPaneStatusBarCtrl m_StatusBar;
	CGlView m_View;

public:
	CGlMainWindow();
	~CGlMainWindow();

	bool InitGlew();
	void Render();

	int OnCreate(CREATESTRUCT *lpcs);
	int OnClose();
	int OnDestroy();

	void OnFileOpen(UINT uNotifyCode, int nID, HWND hWnd);
	void OnFileExit(UINT uNotifyCode, int nID, HWND hWnd);
	void OnSetAnimation(UINT uNotifyCode, int nID, HWND hWnd);
	void OnToggleWireframe(UINT uNotifyCode, int nID, HWND hWnd);
	void OnHelpInfo(UINT uNotifyCode, int nID, HWND hWnd);

	LRESULT OnBenchmark(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (CFrameWindowImpl<CGlMainWindow>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_View.PreTranslateMessage(pMsg);
	}

	DECLARE_FRAME_WND_CLASS_EX(_T("CS_Q3VIEW"), IDR_MAINFRAME, 0, COLOR_WINDOW + 1);

	BEGIN_MSG_MAP_EX(CGlMainWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnFileExit)
		COMMAND_RANGE_HANDLER_EX(ID_BOTH_DEATH1, ID_LEGS_TURN, OnSetAnimation)
		COMMAND_ID_HANDLER_EX(ID_VIEW_TOGGLEWIREFRAME, OnToggleWireframe)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnHelpInfo)

		MESSAGE_HANDLER_EX(WM_APP_BENCHMARK, OnBenchmark)

		CHAIN_MSG_MAP(CFrameWindowImpl<CGlMainWindow>)
	END_MSG_MAP()
};
