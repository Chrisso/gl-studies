#pragma once

#include <vector>
#include <glm/glm.hpp>

struct glyph_desc
{
	glm::ivec2 size;
	glm::ivec2 bearing;
	glm::vec2 tex0;
	glm::vec2 tex1;
	GLfloat advance;
};

class CTextureFont
{
private:
	GLuint m_nTexture = 0;
	GLsizei m_nWidth = 0;
	GLsizei m_nHeight = 0;
	GLsizei m_nFontSize = 0;

	std::vector<glyph_desc> m_Glyphs;

public:
	CTextureFont();
	~CTextureFont();

	bool Create(HINSTANCE hInst, LPCTSTR szResType, int nResId, int nSize = 16);

	operator GLuint() const { return m_nTexture; }
	GLsizei GetWidth() const { return m_nWidth; }
	GLsizei GetHeight() const { return m_nHeight;  }
	GLsizei GetFontSize() const { return m_nFontSize; }
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
