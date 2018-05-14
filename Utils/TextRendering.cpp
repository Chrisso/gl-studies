#include "stdafx.h"
#include <vector>

#include "TextRendering.h"

/////////////////////////////////////////////////////////////////
// CTextureFont - Construction/ Destruction
/////////////////////////////////////////////////////////////////

CTextureFont::CTextureFont()
{
}

CTextureFont::~CTextureFont()
{
	if (m_nTexture)
	{
		glDeleteTextures(1, &m_nTexture);
		m_nTexture = 0;
	}
}

/////////////////////////////////////////////////////////////////
// CTextureFont - Method Implementation
/////////////////////////////////////////////////////////////////

bool CTextureFont::Create()
{
	m_nWidth = 256;
	m_nHeight = 256;

	std::vector<GLubyte> data(m_nWidth*m_nHeight, 32);
	for (int i = 0; i < m_nWidth; i++)
		data[i] = 255;

	for (int i = 0; i < m_nHeight; i++)
		data[i*m_nWidth] = 128;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_nTexture);
	glBindTexture(GL_TEXTURE_2D, m_nTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nWidth, m_nHeight, 0, GL_RED, GL_UNSIGNED_BYTE, &data[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

/////////////////////////////////////////////////////////////////
// CRenderString - Construction/ Destruction
/////////////////////////////////////////////////////////////////

CRenderString::CRenderString()
{

}

CRenderString::~CRenderString()
{
	if (m_nVertexArray)
	{
		glDeleteVertexArrays(1, &m_nVertexArray);
		m_nVertexArray = 0;
	}

	if (m_nVertexCoords)
	{
		glDeleteBuffers(1, &m_nVertexCoords);
		m_nVertexCoords = 0;
	}

	if (m_nTextureCoords)
	{
		glDeleteBuffers(1, &m_nTextureCoords);
		m_nTextureCoords = 0;
	}
}

/////////////////////////////////////////////////////////////////
// CRenderString - Method Implementation
/////////////////////////////////////////////////////////////////

bool CRenderString::Create(const CTextureFont& font)
{
	GLfloat *pData = nullptr;
	GLfloat w = (GLfloat)font.GetWidth();
	GLfloat h = (GLfloat)font.GetHeight();

	glGenVertexArrays(1, &m_nVertexArray);
	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_STATIC_DRAW);

	pData = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	ATLASSERT(pData != nullptr);
	pData[ 0] = 0; pData[ 1] = 0;
	pData[ 2] = 0; pData[ 3] = h;
	pData[ 4] = w; pData[ 5] = h;
	pData[ 6] = w; pData[ 7] = 0;
	pData[ 8] = 0; pData[ 9] = 0;
	pData[10] = w; pData[11] = h;
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &m_nTextureCoords);
	glBindBuffer(GL_ARRAY_BUFFER, m_nTextureCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_STATIC_DRAW);

	pData = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	ATLASSERT(pData != nullptr);
	pData[ 0] = 0; pData[ 1] = 0; // first line in data is bottom of texture
	pData[ 2] = 0; pData[ 3] = 1; //   flip vertically through v' = 1-v
	pData[ 4] = 1; pData[ 5] = 1;
	pData[ 6] = 1; pData[ 7] = 0;
	pData[ 8] = 0; pData[ 9] = 0;
	pData[10] = 1; pData[11] = 1;
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	return true;
}
