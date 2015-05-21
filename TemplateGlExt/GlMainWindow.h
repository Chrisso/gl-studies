#pragma once

#include "resource.h"
#include "Scene.h"

class CGlMainWindow : public CFrameWindowImpl<CGlMainWindow>
{
private:
	HDC   m_hDC;
	HGLRC m_hRC;

	CScene                    *m_pScene;
	CCommandBarCtrl            m_CmdBar;
	CMultiPaneStatusBarCtrl    m_StatusBar;
	Util::CHighResolutionTimer m_Timer;

protected:
	BOOL InitGlew();

public:
	CGlMainWindow();
	~CGlMainWindow();

	void Render();

	LRESULT OnCreate(CREATESTRUCT *lpcs);
	LRESULT OnClose();
	LRESULT OnDestroy();
	LRESULT OnSize(UINT uParam, const CSize& size);

	void OnFileExit(UINT wParam, int lParam, HWND hWnd);
	void OnHelpInfo(UINT wParam, int lParam, HWND hWnd);

	LRESULT OnBenchmark(UINT uMsg, WPARAM wParam, LPARAM lParam);

	DECLARE_FRAME_WND_CLASS_EX(_T("CS_GL_WINDOW32-Template"), IDR_MAINFRAME, CS_DBLCLKS, COLOR_WINDOW);

	BEGIN_MSG_MAP(CGlMainWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)

		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnHelpInfo)

		MESSAGE_HANDLER_EX(WMU_BENCHMARK, OnBenchmark)

		CHAIN_MSG_MAP(CFrameWindowImpl<CGlMainWindow>)
	END_MSG_MAP()
};

