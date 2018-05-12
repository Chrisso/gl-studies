#include "stdafx.h"
#include <initializer_list>
#include "ShaderProgram.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CShaderProgram::CShaderProgram()
{
}

CShaderProgram::~CShaderProgram()
{
	Destroy();
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

void CShaderProgram::Destroy()
{
	if (m_nShaderProgram)
	{
		glDeleteProgram(m_nShaderProgram);
		m_nShaderProgram = 0;
	}
}

bool CShaderProgram::LoadShaderSource(HINSTANCE hInst, LPCTSTR szResType, int nResId, CStringA& shader)
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

bool CShaderProgram::Create(HINSTANCE hInst, LPCTSTR szResType, int nResVertexShader, int nResFragmentShader)
{
	CStringA vertexShaderSource, fragmentShaderSource;
	if (LoadShaderSource(hInst, szResType, nResVertexShader, vertexShaderSource) &&
		LoadShaderSource(hInst, szResType, nResFragmentShader, fragmentShaderSource))
	{
		const GLchar *pSource = vertexShaderSource.GetString();
		GLuint nVS = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(nVS, 1, &pSource, NULL);
		glCompileShader(nVS);

		pSource = fragmentShaderSource.GetString();
		GLuint nFS = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(nFS, 1, &pSource, NULL);
		glCompileShader(nFS);

		GLint statusVS = GL_FALSE;
		glGetShaderiv(nVS, GL_COMPILE_STATUS, &statusVS);

		GLint statusFS = GL_FALSE;
		glGetShaderiv(nFS, GL_COMPILE_STATUS, &statusFS);

		if (statusVS == GL_TRUE && statusFS == GL_TRUE)
		{
			m_nShaderProgram = glCreateProgram();
			glAttachShader(m_nShaderProgram, nVS);
			glAttachShader(m_nShaderProgram, nFS);
			glLinkProgram(m_nShaderProgram);

			GLint status = GL_FALSE;
			glGetProgramiv(m_nShaderProgram, GL_LINK_STATUS, &status);
			if (status != GL_TRUE)
			{
				ATLTRACE(_T("Error linking shader program!\n"));
				glGetProgramiv(m_nShaderProgram, GL_INFO_LOG_LENGTH, &status);

				if (status > 1)
				{
					USES_CONVERSION;
					GLchar *pInfo = new GLchar[status + 1];
					glGetProgramInfoLog(m_nShaderProgram, status + 1, NULL, pInfo);
					m_strLastError = (LPCTSTR)CA2CT(pInfo);
					ATLTRACE(_T("%s\n"), (LPCTSTR)m_strLastError);
					delete[] pInfo;
				}

				glDeleteProgram(m_nShaderProgram);
				m_nShaderProgram = 0;
			}

			glDetachShader(m_nShaderProgram, nFS);
			glDetachShader(m_nShaderProgram, nVS);
		}
		else
		{
			ATLTRACE(_T("Eror compiling shaders!\n"));

			for (GLuint shader : {nVS, nFS})
			{
				GLint status = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &status);
				if (status > 1)
				{
					USES_CONVERSION;
					GLchar *pInfo = new GLchar[status + 1];
					glGetShaderInfoLog(shader, status + 1, NULL, pInfo);
					m_strLastError = (LPCTSTR)CA2CT(pInfo);
					ATLTRACE(_T("%s\n"), (LPCTSTR)m_strLastError);
					delete[] pInfo;
				}
			}
		}

		if (nVS) glDeleteShader(nVS);
		if (nFS) glDeleteShader(nFS);
	}

	return m_nShaderProgram != 0;
}
