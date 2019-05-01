#include "stdafx.h"
#include <vector>
#include "ShaderProgram.h"

/////////////////////////////////////////////////////////////////
// CShader Construction/ Destruction
/////////////////////////////////////////////////////////////////

CShader::CShader()
{
}

CShader::~CShader()
{
	if (m_nShader)
	{
		glDeleteShader(m_nShader);
		m_nShader = 0;
	}
}

/////////////////////////////////////////////////////////////////
// CShader Method Implementation
/////////////////////////////////////////////////////////////////

bool CShader::LoadSource(HINSTANCE hInst, LPCTSTR szResType, int nResId, CStringA& shader)
{
	HRSRC hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(nResId), szResType);
	if (hResInfo)
	{
		HGLOBAL hRes = ::LoadResource(hInst, hResInfo);
		if (hRes)
		{
			LPVOID pResource = ::LockResource(hRes);
			if (pResource)
			{
				shader = CStringA(
					reinterpret_cast<const char*>(pResource),
					::SizeofResource(hInst, hResInfo)
				);
				return true;
			}
		}
	}
	return false;
}

bool CShader::Create(GLenum nType, HINSTANCE hInst, PCTSTR szResType, int nResId)
{
	CStringA strSource;
	if (!LoadSource(hInst, szResType, nResId, strSource))
	{
		ATLTRACE(_T("Error loader shader source from resources (%d)!\n"), nResId);
		return false;
	}

	m_nShader = glCreateShader(nType);
	if (m_nShader == 0)
	{
		ATLTRACE(_T("Error creating shader!\n"));
		return false;
	}

	const GLchar *pSource = strSource.GetString();
	glShaderSource(m_nShader, 1, &pSource, NULL);
	glCompileShader(m_nShader);

	GLint nStatus = GL_FALSE;
	glGetShaderiv(m_nShader, GL_COMPILE_STATUS, &nStatus);

	if (nStatus == GL_FALSE)
	{
		ATLTRACE(_T("Error compiling shader (%d)!\n"), nResId);

		glGetShaderiv(m_nShader, GL_INFO_LOG_LENGTH, &nStatus);
		if (nStatus > 1)
		{
			std::vector<GLchar> info(nStatus + 1);
			glGetShaderInfoLog(m_nShader, (GLsizei)info.size(), NULL, info.data());
			m_strInfoLog = CString(info.data()).Trim();
			ATLTRACE(_T("\n%s\n"), (LPCTSTR)m_strInfoLog);
		}

		glDeleteShader(m_nShader);
		m_nShader = 0;
	}

	return m_nShader != 0;
}

/////////////////////////////////////////////////////////////////
// CShaderProgram Construction/ Destruction
/////////////////////////////////////////////////////////////////

CShaderProgram::CShaderProgram()
{
}

CShaderProgram::~CShaderProgram()
{
	Destroy();
}

/////////////////////////////////////////////////////////////////
// CShaderProgram Method Implementation
/////////////////////////////////////////////////////////////////

bool CShaderProgram::Create()
{
	m_nShaderProgram = glCreateProgram();

	if (m_nShaderProgram == 0)
	{
		ATLTRACE(_T("Error creating shader program!\n"));
		return false;
	}

	return true;
}

void CShaderProgram::Attach(GLuint nShader)
{
	ATLASSERT(nShader != 0);
	ATLASSERT(m_nShaderProgram != 0);
	glAttachShader(m_nShaderProgram, nShader);
}

void CShaderProgram::Detach(GLuint nShader)
{
	ATLASSERT(nShader != 0);
	ATLASSERT(m_nShaderProgram != 0);
	glDetachShader(m_nShaderProgram, nShader);
}

bool CShaderProgram::Link()
{
	ATLASSERT(m_nShaderProgram != 0);
	glLinkProgram(m_nShaderProgram);

	GLint nStatus = GL_FALSE;
	glGetProgramiv(m_nShaderProgram, GL_LINK_STATUS, &nStatus);

	if (nStatus == GL_FALSE)
	{
		ATLTRACE(_T("Error linking shader program!\n"));
		glGetProgramiv(m_nShaderProgram, GL_INFO_LOG_LENGTH, &nStatus);

		if (nStatus > 1)
		{
			std::vector<GLchar> info(nStatus + 1);
			glGetShaderInfoLog(m_nShaderProgram, (GLsizei)info.size(), NULL, info.data());
			m_strInfoLog = CString(info.data()).Trim();
			ATLTRACE(_T("\n%s\n"), (LPCTSTR)m_strInfoLog);
		}
	}

	return nStatus == GL_TRUE;
}

void CShaderProgram::Destroy()
{
	if (m_nShaderProgram)
	{
		glDeleteProgram(m_nShaderProgram);
		m_nShaderProgram = 0;
	}
}

bool CShaderProgram::CreateSimple(HINSTANCE hInst, LPCTSTR szResType, int nResVertexShader, int nResFragmentShader)
{
	CShader vertexShader;
	if (!vertexShader.Create(GL_VERTEX_SHADER, hInst, szResType, nResVertexShader))
	{
		m_strInfoLog = vertexShader.GetInfoLog();
		return false;
	}

	CShader fragmentShader;
	if (!fragmentShader.Create(GL_FRAGMENT_SHADER, hInst, szResType, nResFragmentShader))
	{
		m_strInfoLog = fragmentShader.GetInfoLog();
		return false;
	}

	if (!Create()) return false;

	Attach(vertexShader);
	Attach(fragmentShader);

	bool bResult = Link();

	Detach(fragmentShader);
	Detach(vertexShader);

	if (!bResult) Destroy(); // just to be sure

	return bResult;
}

bool CShaderProgram::Load(LPCTSTR szFile, bool bDeleteOnError)
{
	if (!GLEW_ARB_get_program_binary) // don't need this check if assuming OpenGL 4.1+
		return false;

	HANDLE hFile = ::CreateFile(szFile,
		GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER nSize;
	if (::GetFileSizeEx(hFile, &nSize) && nSize.QuadPart > sizeof(GLenum))
	{
		DWORD dwBytesRead = 0;
		GLenum nFormat;
		if (::ReadFile(hFile, &nFormat, sizeof(GLenum), &dwBytesRead, NULL) &&
			dwBytesRead == sizeof(GLenum))
		{
#ifdef _WIN64
			std::vector<GLbyte> pData(nSize.QuadPart - sizeof(GLenum));
#else
			std::vector<GLbyte> pData(nSize.LowPart - sizeof(GLenum));
#endif
			if (::ReadFile(hFile, pData.data(), (DWORD)pData.size(), &dwBytesRead, NULL) &&
				dwBytesRead == pData.size())
			{
				GLint nStatus = GL_FALSE;
				m_nShaderProgram = glCreateProgram();

				glProgramBinary(m_nShaderProgram, nFormat, pData.data(), (GLsizei)pData.size());			
				glGetProgramiv(m_nShaderProgram, GL_LINK_STATUS, &nStatus);
				if (nStatus == GL_FALSE)
				{
					glDeleteProgram(m_nShaderProgram);
					m_nShaderProgram = 0;
				}
			}
		}
	}

	::CloseHandle(hFile);

	if (m_nShaderProgram == 0 && bDeleteOnError)
		::DeleteFile(szFile);

	return m_nShaderProgram != 0;
}

bool CShaderProgram::Store(LPCTSTR szFile) const
{
	if (!GLEW_ARB_get_program_binary) // don't need this check if assuming OpenGL 4.1+
		return false;

	GLint nSize = 0;
	glGetProgramiv(m_nShaderProgram, GL_PROGRAM_BINARY_LENGTH, &nSize);

	if (nSize == 0) return false;

	GLenum nFormat = GL_NONE;
	std::vector<GLbyte> pData(nSize);
	glGetProgramBinary(m_nShaderProgram, nSize, NULL, &nFormat, pData.data());

	HANDLE hFile = ::CreateFile(szFile,
		GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwBytesWritten = 0;
	::WriteFile(hFile, &nFormat, sizeof(GLenum), &dwBytesWritten, NULL);
	::WriteFile(hFile, pData.data(), nSize, &dwBytesWritten, NULL);
	::CloseHandle(hFile);
	return true;
}
