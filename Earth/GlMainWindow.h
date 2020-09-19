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
	void OnClose();
	void OnDestroy();

	void OnFileExit(UINT uNotifyCode, int nID, HWND hWnd);
	void OnHelpInfo(UINT uNotifyCode, int nID, HWND hWnd);

	LRESULT OnBenchmark(UINT uMsg, WPARAM wParam, LPARAM lParam);

	DECLARE_FRAME_WND_CLASS(_T("CS_EARTHGL"), IDR_MAINFRAME);

	BEGIN_MSG_MAP_EX(CGlMainWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnHelpInfo)

		MESSAGE_HANDLER_EX(WM_APP_BENCHMARK, OnBenchmark)

		CHAIN_MSG_MAP(CFrameWindowImpl<CGlMainWindow>)
	END_MSG_MAP()
};
