#include "stdafx.h"
#include <cstdarg>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "TextRendering.h"

/////////////////////////////////////////////////////////////////
// CTextureFont - Construction/ Destruction
/////////////////////////////////////////////////////////////////

CTextureFont::CTextureFont() : m_Glyphs(256)
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

bool CTextureFont::Create(HINSTANCE hInst, LPCTSTR szResType, int nResId, int nSize)
{
	m_nFontSize = nSize;
	m_nWidth = m_nFontSize * 16;
	m_nHeight = m_nFontSize * 16;

	HRSRC hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(nResId), szResType);
	if (!hResInfo)
	{
		ATLTRACE(_T("Could not find font resource!\n"));
		return false;
	}

	HGLOBAL hRes = ::LoadResource(hInst, hResInfo);
	if (!hRes)
	{
		ATLTRACE(_T("Could not load font resource!\n"));
		return false;
	}

	LPVOID pResource = ::LockResource(hRes);
	if (!pResource)
	{
		ATLTRACE(_T("Could not lock font resource!\n"));
		return false;
	}

	FT_Library pFT;
	if (FT_Init_FreeType(&pFT) != FT_Err_Ok)
	{
		ATLTRACE(_T("Could not initialize freetype library!\n"));
		return false;
	}

	bool bResult = false;

	FT_Face pFace;
	if (FT_New_Memory_Face(pFT, static_cast<FT_Byte*>(pResource), ::SizeofResource(hInst, hResInfo), 0, &pFace) != FT_Err_Ok)
	{
		ATLTRACE(_T("Could not create freetype font from memory!\n"));
		return false;
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_nTexture);
	glBindTexture(GL_TEXTURE_2D, m_nTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nWidth, m_nHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FT_Set_Pixel_Sizes(pFace, 0, m_nFontSize);

	GLfloat fNorm = 1.0f / m_nWidth;
						
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
		{
			wchar_t character = y * 16 + x;
			if (FT_Load_Char(pFace, character, FT_LOAD_RENDER) == FT_Err_Ok)
			{
				glTexSubImage2D(
					GL_TEXTURE_2D, 0,
					x * m_nFontSize, y * m_nFontSize,
					pFace->glyph->bitmap.width, pFace->glyph->bitmap.rows,
					GL_RED, GL_UNSIGNED_BYTE,
					pFace->glyph->bitmap.buffer);

				m_Glyphs[character].advance = pFace->glyph->advance.x / 64.0f;
				m_Glyphs[character].size.x = (GLfloat)pFace->glyph->bitmap.width;
				m_Glyphs[character].size.y = (GLfloat)pFace->glyph->bitmap.rows;
				m_Glyphs[character].bearing.x = (GLfloat)pFace->glyph->bitmap_left;
				m_Glyphs[character].bearing.y = (GLfloat)pFace->glyph->bitmap_top;

				m_Glyphs[character].tex0.x = x * m_nFontSize * fNorm;
				m_Glyphs[character].tex0.y = y * m_nFontSize * fNorm;
				m_Glyphs[character].tex1.x = (x * m_nFontSize + pFace->glyph->bitmap.width) * fNorm;
				m_Glyphs[character].tex1.y = (y * m_nFontSize + pFace->glyph->bitmap.rows) * fNorm;
			}
		}

	FT_Done_Face(pFace);
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

bool CRenderString::CreateFormat(const CTextureFont& font, LPCTSTR szFmt, ...)
{
	CString formatter;

	va_list args;
	va_start(args, szFmt);
	formatter.FormatV(szFmt, args);
	va_end(args);

	return Create(font, (LPCTSTR)formatter);
}

bool CRenderString::Create(const CTextureFont& font, LPCTSTR szText)
{
	CStringW txt(szText);
	int len = txt.GetLength();
	m_nVertices = len * 6;

	GLfloat *pData = nullptr;
	GLfloat w = (GLfloat)font.GetWidth();
	GLfloat h = (GLfloat)font.GetHeight();

	glGenVertexArrays(1, &m_nVertexArray);
	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * m_nVertices, NULL, GL_STATIC_DRAW);

	GLfloat advance = 0.0f;

	pData = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	ATLASSERT(pData != nullptr);
	for (int i = 0; i < len; i++)
	{
		const glyph_desc& gd = font[txt[i] > 255 ? 0 : txt[i]];
		pData[i * 12 +  0] = advance + gd.bearing.x;
		pData[i * 12 +  1] = -gd.bearing.y;
		pData[i * 12 +  2] = advance + gd.bearing.x;
		pData[i * 12 +  3] = -gd.bearing.y + gd.size.y;
		pData[i * 12 +  4] = advance + gd.bearing.x + gd.size.x;
		pData[i * 12 +  5] = -gd.bearing.y + gd.size.y;
		pData[i * 12 +  6] = advance + gd.bearing.x + gd.size.x;
		pData[i * 12 +  7] = -gd.bearing.y;
		pData[i * 12 +  8] = advance + gd.bearing.x;
		pData[i * 12 +  9] = -gd.bearing.y;
		pData[i * 12 + 10] = advance + gd.bearing.x + gd.size.x;
		pData[i * 12 + 11] = -gd.bearing.y + gd.size.y;
		advance += gd.advance;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &m_nTextureCoords);
	glBindBuffer(GL_ARRAY_BUFFER, m_nTextureCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * m_nVertices, NULL, GL_STATIC_DRAW);

	pData = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	ATLASSERT(pData != nullptr);
	for (int i = 0; i < len; i++)
	{
		const glyph_desc& gd = font[txt[i] > 255 ? 0 : txt[i]];
		pData[i * 12 +  0] = gd.tex0.s; pData[i * 12 +  1] = gd.tex0.t;
		pData[i * 12 +  2] = gd.tex0.s; pData[i * 12 +  3] = gd.tex1.t;
		pData[i * 12 +  4] = gd.tex1.s; pData[i * 12 +  5] = gd.tex1.t;
		pData[i * 12 +  6] = gd.tex1.s; pData[i * 12 +  7] = gd.tex0.t;
		pData[i * 12 +  8] = gd.tex0.s; pData[i * 12 +  9] = gd.tex0.t;
		pData[i * 12 + 10] = gd.tex1.s; pData[i * 12 + 11] = gd.tex1.t;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	return true;
}
