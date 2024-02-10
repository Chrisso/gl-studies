#include "pch.h"
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

namespace detail
{
	static void APIENTRY GlDebugProc(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
	{
		USES_CONVERSION;
		reinterpret_cast<const CGlView*>(userParam)->OnDebugMessage(
			source, type, id, severity, CA2CT(message)
		);
	}
}

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
		WGL_DEPTH_BITS_ARB, 24,
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

	int nDebug = GL_NONE;
#ifdef _DEBUG
	if (GLEW_ARB_debug_output) // don't need this check if assuming OpenGL 4.3+
	{
		nDebug = WGL_CONTEXT_DEBUG_BIT_ARB;
	}
#endif // _DEBUG

	int nContextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | nDebug,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	m_hRC = wglCreateContextAttribsARB(m_hDC, NULL, nContextAttributes);
	if (!m_hRC || !wglMakeCurrent(m_hDC, m_hRC))
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

#ifdef _DEBUG
	if (nDebug != GL_NONE)
	{
		glDebugMessageCallback(detail::GlDebugProc, this);
		glDebugMessageInsert(
			GL_DEBUG_SOURCE_APPLICATION,
			GL_DEBUG_TYPE_PERFORMANCE, 0,
			GL_DEBUG_SEVERITY_NOTIFICATION,
			-1, "Using OpenGL debug context.");
	}
#endif // _DEBUG

	if (WGLEW_EXT_swap_control)
	{
		ATLTRACE(_T("Enabling vsync...\n"));
		wglSwapIntervalEXT(WGLEW_EXT_swap_control_tear ? -1 : 1);
	}

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef UNICODE
	int nArgs = 0;
	LPWSTR* szArglist = ::CommandLineToArgvW(::GetCommandLine(), &nArgs);
	if (szArglist && nArgs > 1 &&
		_tcsicmp(::PathFindExtension(szArglist[nArgs - 1]), _T(".pk3")) == 0 &&
		::PathFileExists(szArglist[nArgs - 1]))
	{
		Load(szArglist[nArgs - 1]);
	}
	::LocalFree(szArglist);
#endif

	if (m_pModel == nullptr && ::PathFileExists(_T("tux.pk3")))
		Load(_T("tux.pk3"));

	SetMsgHandled(FALSE);
	return 0;
}

void CGlView::OnDestroy()
{
	ATLTRACE(_T("OnDestroy\n"));

	if (m_pModel)
	{
		delete m_pModel;
		m_pModel = nullptr;
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
}

void CGlView::OnSize(UINT nType, CSize size)
{
	if (m_pModel)
	{
		glViewport(0, 0, size.cx, size.cy);
		m_pModel->Resize(size.cx, size.cy);
	}

	SetMsgHandled(FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Application logic
///////////////////////////////////////////////////////////////////////////////

void CGlView::Load(LPCTSTR szFile)
{
	if (m_pModel) delete m_pModel;

	// check load success, use "Create"
	m_pModel = new Q3Model(szFile);

	// recalc transformations
	CRect rect;
	GetClientRect(&rect);
	OnSize(SIZE_RESTORED, CSize(rect.Width(), rect.Height()));
}

void CGlView::SetAnimation(int id)
{
	if (m_pModel)
		m_pModel->SetAnimation(id);
}

void CGlView::ToggleWireframe()
{
	if (m_pModel)
		m_pModel->SetWireframe(!m_pModel->GetWireframe());
}

void CGlView::Render(float time)
{
	if (!m_hDC || !m_hRC) return;

	glClearColor(0.0f, 0.25f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_pModel)
	{
		m_pModel->Render(time);
	}

	::SwapBuffers(m_hDC);
}

void CGlView::OnDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, LPCTSTR szMsg) const
{
	CString msg;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API: msg.Append(_T("[API]")); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: msg.Append(_T("[Win]")); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: msg.Append(_T("[Compiler]")); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: msg.Append(_T("[3rd-party]")); break;
	case GL_DEBUG_SOURCE_APPLICATION: msg.Append(_T("[App]")); break;
	default: msg.Append(_T("[Other]"));
	}

	msg.Append(_T(" "));

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: msg.Append(_T("HIGH")); break;
	case GL_DEBUG_SEVERITY_MEDIUM: msg.Append(_T("MEDIUM")); break;
	case GL_DEBUG_SEVERITY_LOW: msg.Append(_T("LOW")); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: msg.Append(_T("NOTE")); break;
	}

	msg.Append(_T(" "));

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: msg.Append(_T("Error")); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: msg.Append(_T("Deprecated")); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: msg.Append(_T("Undef")); break;
	case GL_DEBUG_TYPE_PORTABILITY: msg.Append(_T("Portability")); break;
	case GL_DEBUG_TYPE_PERFORMANCE: msg.Append(_T("Performance")); break;
	case GL_DEBUG_TYPE_MARKER: msg.Append(_T("Marker")); break;
	case GL_DEBUG_TYPE_PUSH_GROUP: msg.Append(_T("Push group")); break;
	case GL_DEBUG_TYPE_POP_GROUP: msg.Append(_T("Pop group")); break;
	default: msg.Append(_T("Other"));
	}

	msg.AppendFormat(_T(": %s\n"), szMsg);
	ATLTRACE((LPCTSTR)msg);
}
