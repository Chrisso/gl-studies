#pragma once

#include "Q3Model.h"

class CGlView : public CWindowImpl<CGlView>
{
private:
	HDC      m_hDC = NULL;
	HGLRC    m_hRC = NULL;
	Q3Model *m_pModel = nullptr;

public:
	CGlView();
	~CGlView();

	void Render(float time);
	void Load(LPCTSTR szFile);
	void SetAnimation(int id);
	void ToggleWireframe();

	int OnCreate(CREATESTRUCT *lpcs);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);

	void OnDebugMessage(
		GLenum source, GLenum type, GLuint id,
		GLenum severity, LPCTSTR szMsg) const;

	DECLARE_WND_CLASS_EX(NULL, CS_OWNDC, COLOR_BACKGROUND)

	BEGIN_MSG_MAP_EX(CGlView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()
};
