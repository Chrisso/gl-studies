#include "stdafx.h"
#include "resource.h"
#include "GlView.h"

#define CURL_STATICLIB
#include <curl/curl.h>

#include "Scene.h"
#include "Background.h"

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

	static size_t CurlWriteProc(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
		DWORD dwBytesWritten = 0;
		::WriteFile(reinterpret_cast<HANDLE>(userdata), ptr, (DWORD)(size * nmemb), &dwBytesWritten, NULL);
		return dwBytesWritten;
	}

	DWORD WINAPI DownloadTextureProc(LPVOID pParam)
	{
		bool bResult = false;

		SYSTEMTIME st;
		::GetSystemTime(&st);

		LPCTSTR szUrls[] = {
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73580/world.topo.bathy.200401.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73605/world.topo.bathy.200402.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73630/world.topo.bathy.200403.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73655/world.topo.bathy.200404.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73701/world.topo.bathy.200405.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73726/world.topo.bathy.200406.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73751/world.topo.bathy.200407.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73776/world.topo.bathy.200408.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73801/world.topo.bathy.200409.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73826/world.topo.bathy.200410.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73884/world.topo.bathy.200411.3x5400x2700.jpg"),
			_T("https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73909/world.topo.bathy.200412.3x5400x2700.jpg")
		};

		CString szPng(APP_FILE_T(_tcsrchr(szUrls[st.wMonth - 1], _T('/')) + 1));

		if (!::PathFileExists(szPng))
		{
			CURL *curl = curl_easy_init();
			if (curl)
			{
				HANDLE hFile = ::CreateFile(
					szPng,
					GENERIC_WRITE, 0, NULL,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					USES_CONVERSION;
					ATLTRACE(_T("Downloading texture to %s...\n"), (LPCTSTR)szPng);
					curl_easy_setopt(curl, CURLOPT_URL, (const char*)CT2CA(szUrls[st.wMonth - 1]));
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, detail::CurlWriteProc);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, hFile);
					CURLcode code = curl_easy_perform(curl);
					if (code != CURLE_OK)
					{
						ATLTRACE(_T("Curl error: %s\n"), (LPCTSTR)CA2CT(curl_easy_strerror(code)));
					}
					bResult = curl_easy_perform(curl) == CURLE_OK;
					::CloseHandle(hFile);

					if (!bResult) ::DeleteFile(szPng);
				}
				curl_easy_cleanup(curl);
			}
		}
		else bResult = true;

		if (bResult)
			::SendMessage(reinterpret_cast<HWND>(pParam),
				WM_APP_RESOURCE_READY,
				IDR_IMAGE_BLUE_MARBLE,
				(LPARAM)(LPCTSTR)szPng);

		return 0;
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

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	m_pScene = new CSceneGraphNode();
	if (!m_pScene->Create())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}

	CBackground *pBackground = new CBackground();
	if (!pBackground->Create())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}
	m_pScene->AddChild(pBackground);

	CScene *pMainScene = new CScene();
	if (!pMainScene->Create())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_OPENGL, IDR_MAINFRAME);
		return -1;
	}
	m_pScene->AddChild(pMainScene);

	if (curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK)
	{
		::CreateThread(NULL, 0, detail::DownloadTextureProc, m_hWnd, 0, NULL);
	}
	else ATLTRACE(_T("Could not initialize curl!\n"));

	SetMsgHandled(FALSE);
	return 0;
}

int CGlView::OnDestroy()
{
	ATLTRACE(_T("OnDestroy\n"));

	if (m_pScene)
	{
		delete m_pScene;
		m_pScene = nullptr;
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

LRESULT CGlView::OnResourceReady(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_pScene->ResourceReady((int)wParam, (void*)lParam);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Application logic
///////////////////////////////////////////////////////////////////////////////

void CGlView::Render(float time)
{
	if (!m_hDC || !m_hRC) return;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_pScene)
	{
		m_pScene->Render(time);
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
