#include "stdafx.h"
#include "resource.h"
#include "GlView.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CGlView::CGlView()
{
}

CGlView::~CGlView()
{
}

/////////////////////////////////////////////////////////////////
// Method Implementation/ Windows Message Handling
/////////////////////////////////////////////////////////////////

int CGlView::OnCreate(CREATESTRUCT *lpcs)
{
	ATLTRACE(_T("OnCreate\n"));

	m_hDC = GetDC();
	if (!m_hDC)
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

	SetMsgHandled(FALSE);
	return 0;
}

int CGlView::OnDestroy()
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

	return 0;
}

int CGlView::OnSize(UINT nType, CSize size)
{
	if (m_pScene)
	{
		glViewport(0, 0, size.cx, size.cy);
		m_pScene->Resize(size.cx, size.cy);
	}

	SetMsgHandled(FALSE);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Application logic
///////////////////////////////////////////////////////////////////////////////

void CGlView::Render(float time)
{
	if (!m_hDC || !m_hRC) return;

	if (m_pScene)
	{
		m_pScene->Render(time);
	}

	::SwapBuffers(m_hDC);
}
