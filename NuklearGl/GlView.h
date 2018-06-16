#pragma once

#include "Scene.h"

class CGlView : public CWindowImpl<CGlView>
{
private:
	HDC     m_hDC = NULL;
	HGLRC   m_hRC = NULL;
	CScene *m_pScene = nullptr;

public:
	CGlView();
	~CGlView();

	void Render(float time);

	int OnCreate(CREATESTRUCT *lpcs);
	int OnDestroy();
	int OnSize(UINT nType, CSize size);

	void OnDebugMessage(
		GLenum source, GLenum type, GLuint id,
		GLenum severity, LPCTSTR szMsg) const;

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (m_pScene) m_pScene->MouseMove(nFlags, (int)point.x, (int)point.y);
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		if (m_pScene) m_pScene->MouseButton(MK_LBUTTON, true, (int)point.x, (int)point.y);
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		if (m_pScene) m_pScene->MouseButton(MK_LBUTTON, false, (int)point.x, (int)point.y);
	}

	void OnChar(TCHAR chChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_pScene) m_pScene->Char((char)chChar, nFlags);
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_pScene) m_pScene->Key(nChar, nFlags, true);
	}

	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_pScene) m_pScene->Key(nChar, nFlags, false);
	}

	BOOL PreTranslateMessage(MSG*)
	{
		return FALSE;
	}

	DECLARE_WND_CLASS_EX(NULL, CS_OWNDC, 0)

	BEGIN_MSG_MAP_EX(CGlView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)

		MSG_WM_MOUSEMOVE(OnMouseMove);
		MSG_WM_LBUTTONDOWN(OnLButtonDown);
		MSG_WM_LBUTTONUP(OnLButtonUp);
		MSG_WM_CHAR(OnChar);
		MSG_WM_KEYDOWN(OnKeyDown);
		MSG_WM_KEYUP(OnKeyUp);
	END_MSG_MAP()
};
