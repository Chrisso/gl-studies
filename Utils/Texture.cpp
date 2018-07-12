#include "stdafx.h"
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include <stb_image.h>

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CTexture::CTexture()
{
	glGenTextures(1, &m_nTexture);
}

CTexture::~CTexture()
{
	if (m_nTexture)
	{
		glDeleteTextures(1, &m_nTexture);
		m_nTexture = 0;
	}
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

bool CTexture::LoadInternal(GLubyte *pData, bool bHiqual)
{
	glBindTexture(GL_TEXTURE_2D, m_nTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_nWidth, m_nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (bHiqual)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		if (GLEW_ARB_texture_filter_anisotropic)
		{
			float fMaxAniso = 0.0;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &fMaxAniso);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, fMaxAniso);
		}
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool CTexture::Load(HINSTANCE hInst, LPCTSTR szResType, int nResId, bool bHiqual)
{
	if (m_nWidth != 0 && m_nHeight != 0) Reset();

	ATLASSERT(m_nTexture != 0);

	HRSRC hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(nResId), szResType);
	if (!hResInfo)
	{
		ATLTRACE(_T("Could not find image resource!\n"));
		return false;
	}

	HGLOBAL hRes = ::LoadResource(hInst, hResInfo);
	if (!hRes)
	{
		ATLTRACE(_T("Could not load image resource!\n"));
		return false;
	}

	LPVOID pResource = ::LockResource(hRes);
	if (!pResource)
	{
		ATLTRACE(_T("Could not lock image resource!\n"));
		return false;
	}

	int nChannels = 0;
	GLubyte *pData = stbi_load_from_memory(
		reinterpret_cast<const stbi_uc*>(pResource),
		::SizeofResource(hInst, hResInfo),
		&m_nWidth, &m_nHeight, &nChannels, 4);

	if (!pData)
	{
		ATLTRACE(_T("Could not decode image resource!\n"));
		return false;
	}

	LoadInternal(pData, bHiqual);

	stbi_image_free(pData);
	return true;
}

bool CTexture::Load(LPCTSTR szFileName, bool bHiqual)
{
	if (m_nWidth != 0 && m_nHeight != 0) Reset();

	ATLASSERT(m_nTexture != 0);

	USES_CONVERSION;

	int nChannels = 0;
	GLubyte *pData = stbi_load(CT2CA(szFileName), &m_nWidth, &m_nHeight, &nChannels, 4);

	if (!pData)
	{
		ATLTRACE(_T("Could not decode image file!\n"));
		return false;
	}

	LoadInternal(pData, bHiqual);

	stbi_image_free(pData);
	return true;
}

void CTexture::Reset()
{
	glDeleteTextures(1, &m_nTexture);
	glGenTextures(1, &m_nTexture);
	m_nWidth = m_nHeight = 0;
}
