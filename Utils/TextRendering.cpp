#include "stdafx.h"
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

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_nTexture);
	glBindTexture(GL_TEXTURE_2D, m_nTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nWidth, m_nHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	bool bResult = false;
	HRSRC hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(nResId), szResType);
	if (hResInfo)
	{
		HGLOBAL hRes = ::LoadResource(hInst, hResInfo);
		if (hRes)
		{
			LPVOID pResource = ::LockResource(hRes);
			if (pResource)
			{
				FT_Library pFT;
				if (FT_Init_FreeType(&pFT) == FT_Err_Ok)
				{
					FT_Face pFace;
					if (FT_New_Memory_Face(pFT, static_cast<FT_Byte*>(pResource), ::SizeofResource(hInst, hResInfo), 0, &pFace) == FT_Err_Ok)
					{
						ATLTRACE(_T("Freetype ready\n"));
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
										x * 16, y * 16,
										pFace->glyph->bitmap.width, pFace->glyph->bitmap.rows,
										GL_RED, GL_UNSIGNED_BYTE,
										pFace->glyph->bitmap.buffer);

									m_Glyphs[character].advance = pFace->glyph->advance.x * 64.0f;
									m_Glyphs[character].size.x = pFace->glyph->bitmap.width;
									m_Glyphs[character].size.y = pFace->glyph->bitmap.rows;
									m_Glyphs[character].bearing.x = pFace->glyph->bitmap_left;
									m_Glyphs[character].bearing.y = pFace->glyph->bitmap_top;

									m_Glyphs[character].tex0.x = x * 16 * fNorm;
									m_Glyphs[character].tex0.y = y * 16 * fNorm;
									m_Glyphs[character].tex1.x = (x * 16 + pFace->glyph->bitmap.width) * fNorm;
									m_Glyphs[character].tex1.y = (y * 16 + pFace->glyph->bitmap.rows) * fNorm;
								}
							}

						FT_Done_Face(pFace);
						bResult = true;
					}
				}
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return bResult;
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
