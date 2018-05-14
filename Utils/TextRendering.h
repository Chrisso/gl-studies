#pragma once

class CTextureFont
{
private:
	GLuint m_nTexture = 0;
	GLsizei m_nWidth = 0;
	GLsizei m_nHeight = 0;

public:
	CTextureFont();
	~CTextureFont();

	bool Create();

	operator GLuint() const { return m_nTexture; }
	GLsizei GetWidth() const { return m_nWidth; }
	GLsizei GetHeight() const { return m_nHeight;  }
};

class CRenderString
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexCoords = 0;
	GLuint m_nTextureCoords = 0;

public:
	CRenderString();
	~CRenderString();

	bool Create(const CTextureFont& font);

	operator GLuint() const { return m_nVertexArray; }
};
