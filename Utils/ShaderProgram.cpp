#include "stdafx.h"
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
			USES_CONVERSION;
			GLchar *pInfo = new GLchar[nStatus + 1];
			glGetShaderInfoLog(m_nShader, nStatus + 1, NULL, pInfo);
			m_strLastError = (LPCTSTR)CA2CT(pInfo);
			ATLTRACE(_T("%s\n"), (LPCTSTR)m_strLastError);
			delete[] pInfo;
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
			USES_CONVERSION;
			GLchar *pInfo = new GLchar[nStatus + 1];
			glGetProgramInfoLog(m_nShaderProgram, nStatus + 1, NULL, pInfo);
			m_strLastError = (LPCTSTR)CA2CT(pInfo);
			ATLTRACE(_T("%s\n"), (LPCTSTR)m_strLastError);
			delete[] pInfo;
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
		m_strLastError = vertexShader.GetLastError();
		return false;
	}

	CShader fragmentShader;
	if (!fragmentShader.Create(GL_FRAGMENT_SHADER, hInst, szResType, nResFragmentShader))
	{
		m_strLastError = fragmentShader.GetLastError();
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
