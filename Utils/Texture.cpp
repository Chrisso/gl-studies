#include "stdafx.h"
#include <stdint.h>
#include <atlfile.h>
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

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

	if (!bHiqual)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else Upgrade();

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void CTexture::Upgrade()
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

	if (_tcsicmp(::PathFindExtension(szFileName), _T(".ktx")) == 0)
		return LoadKTX(szFileName, bHiqual);

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

namespace detail
{
	struct ktx_info_t
	{
		uint32_t glType;
		uint32_t glTypeSize;
		uint32_t glFormat;
		uint32_t glInternalFormat;
		uint32_t glBaseInternalFormat;
		uint32_t pixelWidth;
		uint32_t pixelHeight;
		uint32_t pixelDepth;
		uint32_t numberOfArrayElements;
		uint32_t numberOfFaces;
		uint32_t numberOfMipmapLevels;
	};
}

bool CTexture::LoadKTX(LPCTSTR szFileName, bool bHiqual)
{
	CAtlFile file;
	if (FAILED(file.Create(szFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)))
	{
		ATLTRACE(_T("KTX file not opened: %s\n"), szFileName);
		return false;
	}
	
	DWORD dwBytesRead;

	uint8_t magic_ref[12] = {
		0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
	};

	uint8_t magic[12];
	if (FAILED(file.Read(magic, 12, dwBytesRead)) ||
		dwBytesRead != 12 ||
		memcmp(magic_ref, magic, 12) != 0)
	{
		ATLTRACE(_T("KTX signature invalid.\n"));
		return false;
	}

	uint32_t endianness;
	if (FAILED(file.Read(&endianness, sizeof(uint32_t), dwBytesRead)) ||
		dwBytesRead != sizeof(uint32_t) ||
		endianness != 0x04030201)
	{
		ATLTRACE(_T("KTX endianness not supported or invalid.\n"));
		return false;
	}

	detail::ktx_info_t info;
	if (FAILED(file.Read(&info, sizeof(detail::ktx_info_t), dwBytesRead)) ||
		dwBytesRead != sizeof(detail::ktx_info_t) ||
		info.glType != GL_UNSIGNED_BYTE ||
		info.pixelHeight == 0 || // no 1D texture
		info.pixelDepth != 0 || // no 3D texture
		info.numberOfArrayElements != 0 || // no array textures
		info.numberOfFaces != 1 // no cube maps
	)
	{
		ATLTRACE(_T("KTX data format not supported by this implementation.\n"));
		return false;
	}

	uint32_t bytesOfKeyValueData;
	if (FAILED(file.Read(&bytesOfKeyValueData, sizeof(uint32_t), dwBytesRead)) ||
		dwBytesRead != sizeof(uint32_t))
	{
		ATLTRACE(_T("KTX key-value spec invalid.\n"));
		return false;
	}

	if (FAILED(file.Seek(bytesOfKeyValueData)))
	{
		ATLTRACE(_T("KTX key-value skip failed.\n"));
		return false;
	}

	uint32_t imageSize;
	if (FAILED(file.Read(&imageSize, sizeof(uint32_t), dwBytesRead)) ||
		dwBytesRead != sizeof(uint32_t) ||
		imageSize == 0)
	{
		ATLTRACE(_T("KTX image size invalid.\n"));
		return false;
	}

	bool bResult = false;
	GLubyte *pData = new GLubyte[imageSize];
	if (SUCCEEDED(file.Read(pData, imageSize, dwBytesRead)) && dwBytesRead == imageSize)
	{
		m_nWidth = info.pixelWidth;
		m_nHeight = info.pixelHeight;

		glBindTexture(GL_TEXTURE_2D, m_nTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, info.glInternalFormat,
			m_nWidth, m_nHeight, 0, info.glFormat, info.glType, pData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (!bHiqual)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else Upgrade();

		bResult = true;
	}
	else
	{
		ATLTRACE(_T("KTX image data invalid.\n"));
	}

	delete[] pData;

	return bResult;
}

bool CTexture::Store(LPCTSTR szFileName, GLubyte *data, GLsizei width, GLsizei height)
{
	uint8_t magic[12] = {
		0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
	};

	uint32_t endianness = 0x04030201;
	uint32_t bytesOfKeyValueData = 0;
	uint32_t imageSize = width * height * 4 * sizeof(GLubyte);

	detail::ktx_info_t info;
	info.glType = GL_UNSIGNED_BYTE;
	info.glTypeSize = 1;
	info.glFormat = GL_RGBA;
	info.glInternalFormat = info.glBaseInternalFormat = GL_RGBA;
	info.pixelWidth = width;
	info.pixelHeight = height;
	info.pixelDepth = 0;
	info.numberOfArrayElements = 0;
	info.numberOfFaces = 1;
	info.numberOfMipmapLevels = 1;

	HANDLE hFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwBytesWritten;

	::WriteFile(hFile, magic, 12, &dwBytesWritten, NULL);
	::WriteFile(hFile, &endianness, sizeof(uint32_t), &dwBytesWritten, NULL);
	::WriteFile(hFile, &info, sizeof(detail::ktx_info_t), &dwBytesWritten, NULL);
	::WriteFile(hFile, &bytesOfKeyValueData, sizeof(uint32_t), &dwBytesWritten, NULL);
	::WriteFile(hFile, &imageSize, sizeof(uint32_t), &dwBytesWritten, NULL);
	::WriteFile(hFile, data, imageSize, &dwBytesWritten, NULL);

	::CloseHandle(hFile);

	return true;
}
