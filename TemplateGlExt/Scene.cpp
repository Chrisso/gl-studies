#include "stdafx.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AppFile.h>

#include "resource.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////
// CScene Construction/ Destruction
/////////////////////////////////////////////////////////////////

CScene::CScene()
{
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

	if (!APP_FILE_EXISTS("default.shader"))
	{
		ATLTRACE(_T("Saving compiled default shader to file.\n"));
		m_ShaderProgram.Store(APP_FILE("default.shader"));
	}
}

/////////////////////////////////////////////////////////////////
// CScene Method Implementation
/////////////////////////////////////////////////////////////////

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

	if (!m_ShaderProgram.Load(APP_FILE("default.shader")))
	{
		ATLTRACE(_T("Could not restore compiled default shader. Recompiling...\n"));
		if (!m_ShaderProgram.CreateSimple(
			_Module.GetResourceInstance(), _T("GLSL_SHADER"),
			IDR_GLSL_VERTEX_SHADER,
			IDR_GLSL_FRAGMENT_SHADER))
		{
			// trigger immediate window destruction and thus cascading destructor
			return false;
		}
	}

	return true;
}

void CScene::Render(float time)
{
	glUseProgram(m_ShaderProgram);

	m_matMVP = glm::rotate(
		m_matMVP,
		glm::radians(-time / 20.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glUniformMatrix4fv(
		glGetUniformLocation(m_ShaderProgram, "transformation"),
		1, GL_FALSE, glm::value_ptr(m_matMVP)
	);

	if (m_nVertexArray && m_nVertexBuffer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
		glBindVertexArray(m_nVertexArray);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(0);
}

void CScene::Resize(int width, int height)
{
	if (height == 0) height = 1; // avoid division by zero

	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		(float)width / (float)height,
		1.0f, 32.0f);

	glm::mat4 modelview = glm::lookAt(
		glm::vec3(0.0f, 2.0f, 5.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	m_matMVP = projection * modelview;
}

/////////////////////////////////////////////////////////////////
// CHud Construction/ Destruction
/////////////////////////////////////////////////////////////////

CHud::CHud()
{
}

CHud::~CHud()
{
	if (!APP_FILE_EXISTS("font.shader"))
	{
		ATLTRACE(_T("Saving compiled font shader to file.\n"));
		m_ShaderProgram.Store(APP_FILE("font.shader"));
	}
}

/////////////////////////////////////////////////////////////////
// CHud Method Implementation
/////////////////////////////////////////////////////////////////

bool CHud::Create()
{
	ATLTRACE(_T("Initializing...\n"));

	if (!m_ShaderProgram.Load(APP_FILE("font.shader")))
	{
		ATLTRACE(_T("Could not restore compiled font shader. Recompiling...\n"));
		if (!m_ShaderProgram.CreateSimple(
			_Module.GetResourceInstance(), _T("GLSL_SHADER"),
			IDR_GLSL_TEXTVERTEX_SHADER,
			IDR_GLSL_TEXTFRAGMENT_SHADER))
			return false;
	}

	if (!m_TextureFont.Create(_Module.GetResourceInstance(), RT_FONT, IDR_FONT_OPENSANS, 12))
	{
		ATLTRACE(_T("Error initializing texture font!\n"));
		return false;
	}

	USES_CONVERSION;

	if (!m_RenderString.CreateFormat(
		m_TextureFont,
		_T("OpenGL %s"),
		(LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_VERSION)))))
	{
		ATLTRACE(_T("Error creating text rendering resources!\n"));
		return false;
	}

	return true;
}

void CHud::Render(float time)
{
	glUseProgram(m_ShaderProgram);

	glm::mat4 matText = glm::translate(m_matMVP, glm::vec3(10.0f, 32.0f, 0.0f));

	glUniformMatrix4fv(
		glGetUniformLocation(m_ShaderProgram, "transformation"),
		1, GL_FALSE, glm::value_ptr(matText)
	);

	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, m_TextureFont);
	glBindVertexArray(m_RenderString);
	glDrawArrays(GL_TRIANGLES, 0, m_RenderString.NumVertices());
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(0);
}

void CHud::Resize(int width, int height)
{
	m_matMVP = glm::ortho(0.0f, (GLfloat)width, (GLfloat)height, 0.0f);
}
