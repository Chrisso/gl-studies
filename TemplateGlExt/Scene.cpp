#include "stdafx.h"
#include <initializer_list>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "resource.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CScene::CScene()
{
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
}

CScene::~CScene()
{
	ATLTRACE(_T("Cleaning up...\n"));

	if (m_nVertexArray)
	{
		glDeleteVertexArrays(1, &m_nVertexArray);
		m_nVertexArray = 0;
	}

	if (m_nVertexBuffer)
	{
		glDeleteBuffers(1, &m_nVertexBuffer);
		m_nVertexBuffer = 0;
	}

	if (m_nShaderProgram)
	{
		glDeleteProgram(m_nShaderProgram);
		m_nShaderProgram = 0;
	}
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

bool CScene::LoadShaderSource(int id, CStringA& shader)
{
	HINSTANCE hInst = _Module.GetResourceInstance();
	HRSRC hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(id), _T("GLSL_SHADER"));
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

bool CScene::Create()
{
	ATLTRACE(_T("Initializing...\n"));

	glGenVertexArrays(1, &m_nVertexArray);
	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexBuffer);
	if (!m_nVertexBuffer)
		return false;

	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, NULL, GL_STATIC_DRAW);

	float *pGeometry = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	pGeometry[0] = 0.0f; pGeometry[1] = 1.0f; pGeometry[2] = 0.0f;
	pGeometry[3] = -1.0f; pGeometry[4] = 0.0f; pGeometry[5] = -1.0f;
	pGeometry[6] = 1.0f; pGeometry[7] = 0.0f; pGeometry[8] = -1.0f;
	pGeometry[9] = 1.0f; pGeometry[10] = 0.0f; pGeometry[11] = 1.0f;
	pGeometry[12] = -1.0f; pGeometry[13] = 0.0f; pGeometry[14] = 1.0f;
	pGeometry[15] = -1.0f; pGeometry[16] = 0.0f; pGeometry[17] = -1.0f;
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	CStringA vertexShaderSource, fragmentShaderSource;
	if (LoadShaderSource(IDR_GLSL_VERTEX_SHADER, vertexShaderSource) &&
		LoadShaderSource(IDR_GLSL_FRAGMENT_SHADER, fragmentShaderSource))
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
			if (status == GL_TRUE)
			{
				ATLTRACE(_T("Rendering with shaders.\n"));
				glUseProgram(m_nShaderProgram);
			}
			else
			{
				ATLTRACE(_T("Error linking shader program!\n"));
				glGetProgramiv(m_nShaderProgram, GL_INFO_LOG_LENGTH, &status);

				if (status > 1)
				{
					USES_CONVERSION;
					GLchar *pInfo = new GLchar[status + 1];
					glGetProgramInfoLog(m_nShaderProgram, status + 1, NULL, pInfo);
					ATLTRACE(_T("%s\n"), (LPCTSTR)CA2CT(pInfo));
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
					ATLTRACE(_T("%s\n"), (LPCTSTR)CA2CT(pInfo));
					delete[] pInfo;
				}
			}
		}

		if (nVS) glDeleteShader(nVS);
		if (nFS) glDeleteShader(nFS);
	}

	return m_nShaderProgram != 0;
}

void CScene::Render(float time)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_matMVP = glm::rotate(
		m_matMVP,
		glm::radians(-time / 20.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glUniformMatrix4fv(
		glGetUniformLocation(m_nShaderProgram, "transformation"),
		1, GL_FALSE, glm::value_ptr(m_matMVP)
	);

	if (m_nVertexArray && m_nVertexBuffer)
	{
		glBindVertexArray(m_nVertexArray);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
		glBindVertexArray(0);
	}
}

void CScene::Resize(int width, int height)
{
	if (height == 0) height = 1; // avoid division by zero

	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.0f, 32.0f);
	glm::mat4 modelview = glm::lookAt(
		glm::vec3(0.0f, 2.0f, -5.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	m_matMVP = projection * modelview;
}
