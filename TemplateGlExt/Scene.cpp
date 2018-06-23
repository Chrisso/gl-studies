#include "stdafx.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AppFile.h>

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

	if (m_pRenderString)
	{
		delete m_pRenderString;
		m_pRenderString = nullptr;
	}

	if (m_pTextureFont)
	{
		delete m_pTextureFont;
		m_pTextureFont = nullptr;
	}

	if (m_pFontShader)
	{
		if (!APP_FILE_EXISTS("font.shader"))
		{
			ATLTRACE(_T("Saving compiled font shader to file.\n"));
			m_pFontShader->Store(APP_FILE("font.shader"));
		}

		delete m_pFontShader;
		m_pFontShader = nullptr;
	}

	if (m_pShaderProgram)
	{
		if (!APP_FILE_EXISTS("default.shader"))
		{
			ATLTRACE(_T("Saving compiled default shader to file.\n"));
			m_pShaderProgram->Store(APP_FILE("default.shader"));
		}

		delete m_pShaderProgram;
		m_pShaderProgram = nullptr;
	}
}

/////////////////////////////////////////////////////////////////
// Method Implementation
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

	m_pShaderProgram = new CShaderProgram();
	if (!m_pShaderProgram->Load(APP_FILE("default.shader")))
	{
		ATLTRACE(_T("Could not restore compiled default shader. Recompiling...\n"));
		if (!m_pShaderProgram->CreateSimple(
			_Module.GetResourceInstance(), _T("GLSL_SHADER"),
			IDR_GLSL_VERTEX_SHADER,
			IDR_GLSL_FRAGMENT_SHADER))
		{
			// trigger immediate window destruction and thus cascading destructor
			return false;
		}
	}

	m_pFontShader = new CShaderProgram();
	if (!m_pFontShader->Load(APP_FILE("font.shader")))
	{
		ATLTRACE(_T("Could not restore compiled font shader. Recompiling...\n"));
		if (!m_pFontShader->CreateSimple(
			_Module.GetResourceInstance(), _T("GLSL_SHADER"),
			IDR_GLSL_TEXTVERTEX_SHADER,
			IDR_GLSL_TEXTFRAGMENT_SHADER))
			return false;
	}

	m_pTextureFont = new CTextureFont();
	if (!m_pTextureFont->Create(_Module.GetResourceInstance(), RT_FONT, IDR_FONT_OPENSANS, 12))
	{
		ATLTRACE(_T("Error initializing texture font!\n"));
		return false;
	}

	USES_CONVERSION;

	m_pRenderString = new CRenderString();
	if (!m_pRenderString->CreateFormat(
		*m_pTextureFont,
		_T("OpenGL %s"),
		(LPCTSTR)CA2CT(reinterpret_cast<const char*>(glGetString(GL_VERSION)))))
	{
		ATLTRACE(_T("Error creating text rendering resources!\n"));
		return false;
	}

	return true;
}

void CScene::Render(float time)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(*m_pShaderProgram);

	m_matSceneMVP = glm::rotate(
		m_matSceneMVP,
		glm::radians(-time / 20.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glUniformMatrix4fv(
		glGetUniformLocation(*m_pShaderProgram, "transformation"),
		1, GL_FALSE, glm::value_ptr(m_matSceneMVP)
	);

	if (m_nVertexArray && m_nVertexBuffer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
		glBindVertexArray(m_nVertexArray);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(*m_pFontShader);

	glm::mat4 matText = glm::translate(m_matHudMVP, glm::vec3(10.0f, 32.0f, 0.0f));

	glUniformMatrix4fv(
		glGetUniformLocation(*m_pFontShader, "transformation"),
		1, GL_FALSE, glm::value_ptr(matText)
	);

	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, *m_pTextureFont);
	glBindVertexArray(*m_pRenderString);
	glDrawArrays(GL_TRIANGLES, 0, m_pRenderString->NumVertices());
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
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

	m_matSceneMVP = projection * modelview;

	m_matHudMVP = glm::ortho(0.0f, (GLfloat)width, (GLfloat)height, 0.0f);
}
