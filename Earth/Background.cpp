#include "stdafx.h"
#include "Background.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CBackground::CBackground()
{
}

CBackground::~CBackground()
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
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

bool CBackground::Create()
{
	if (!m_ShaderProgram.CreateSimple(
		_Module.GetResourceInstance(),
		_T("GLSL_SHADER"),
		IDR_GLSL_VERTEX_PASSTHROUGH,
		IDR_GLSL_FRAGMENT_TEXTURED))
		return false;

	if (m_Texture == 0 || !m_Texture.Load(
		_Module.GetResourceInstance(), _T("IMAGE_JPEG"), IDR_IMAGE_SPACE))
		return false;

	glGenVertexArrays(1, &m_nVertexArray);
	if (!m_nVertexArray) return false;
	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexBuffer);
	if (!m_nVertexBuffer) return false;
	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 4, NULL, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Resize(800, 600); // create some initial data

	return true;
}

void CBackground::Resize(int width, int height)
{
	// assumes repeat as texture wrap mode
	float pGeometry[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, height/(float)m_Texture.GetHeight(),
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, width / (float)m_Texture.GetWidth(), height / (float)m_Texture.GetHeight(),
		 1.0f, -1.0f, 0.0f, width / (float)m_Texture.GetWidth(), 0.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 5 * 4, pGeometry);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CBackground::Render(float time)
{
	if (!m_nVertexArray || !m_nVertexBuffer)
		return;

	glDisable(GL_DEPTH_TEST);
	glUseProgram(m_ShaderProgram);
	glBindTexture(GL_TEXTURE_2D, m_Texture);

	glBindVertexArray(m_nVertexArray);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}
