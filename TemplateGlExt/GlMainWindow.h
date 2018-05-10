#pragma once

#include <atlctrlw.h>
#include <atlctrlx.h>
#include "Resource.h"

#include "FrameCounter.h"
#include "Scene.h"

class CGlMainWindow : public CFrameWindowImpl<CGlMainWindow>
{
private:
	HDC   m_hDC = NULL;
	HGLRC m_hRC = NULL;

	CScene *m_pScene = nullptr;
	CFrameCounter m_FrameCounter;
	CCommandBarCtrl m_CmdBar;
	CMultiPaneStatusBarCtrl m_StatusBar;

public:
	CGlMainWindow();
	~CGlMainWindow();

	bool InitGlew();
	void Render();

	int OnCreate(CREATESTRUCT *lpcs);
	int OnClose();
	int OnDestroy();
	int OnSize(UINT nType, CSize size);

	void OnFileExit(UINT uNotifyCode, int nID, HWND hWnd);
	void OnHelpInfo(UINT uNotifyCode, int nID, HWND hWnd);

	LRESULT OnBenchmark(UINT uMsg, WPARAM wParam, LPARAM lParam);

	DECLARE_FRAME_WND_CLASS_EX(_T("CS_TEMPLATEGL"), IDR_MAINFRAME, CS_OWNDC | CS_DBLCLKS, COLOR_WINDOW + 1);

	BEGIN_MSG_MAP_EX(CGlMainWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)

		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnHelpInfo)

		MESSAGE_HANDLER_EX(WM_APP_BENCHMARK, OnBenchmark)

		CHAIN_MSG_MAP(CFrameWindowImpl<CGlMainWindow>)
	END_MSG_MAP()
};
