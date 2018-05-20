#pragma once

#include <vector>
#include <glm/glm.hpp>

struct glyph_desc
{
	glm::vec2 size;
	glm::vec2 bearing;
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
	GLsizei m_nTextHeight = 0;

	std::vector<glyph_desc> m_Glyphs;

public:
	CTextureFont();
	~CTextureFont();

	bool Create(HINSTANCE hInst, LPCTSTR szResType, int nResId, int nSize = 16);

	operator GLuint() const { return m_nTexture; }
	GLsizei GetWidth() const { return m_nWidth; }
	GLsizei GetHeight() const { return m_nHeight;  }
	GLsizei GetTextHeight() const { return m_nTextHeight; }

	const glyph_desc& operator[](size_t n) const { return m_Glyphs[n]; }
};

class CRenderString
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexCoords = 0;
	GLuint m_nTextureCoords = 0;
	GLsizei m_nVertices = 0;
	GLfloat m_fTextWidth = 0.0f;

public:
	CRenderString();
	~CRenderString();

	bool CreateFormat(const CTextureFont& font, LPCTSTR szFmt, ...);
	bool Create(const CTextureFont& font, LPCTSTR szText);

	operator GLuint() const { return m_nVertexArray; }
	GLsizei NumVertices() const { return m_nVertices; }
	GLfloat GetTextWidth() const { return m_fTextWidth; }
};
