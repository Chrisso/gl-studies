#pragma once

class CTexture
{
private:
	GLuint m_nTexture = 0;
	GLsizei m_nWidth = 0;
	GLsizei m_nHeight = 0;

	bool LoadInternal(GLubyte *pData, bool bHiqual);
	bool LoadKTX(LPCTSTR szFileName, bool bHiqual);
	void Upgrade();

public:
	CTexture();
	~CTexture();

	operator GLuint() const { return m_nTexture; }
	GLsizei GetWidth() const { return m_nWidth; }
	GLsizei GetHeight() const { return m_nHeight; }

	bool Load(HINSTANCE hInst, LPCTSTR szResType, int nResId, bool bHiqual = false);
	bool Load(LPCTSTR szFileName, bool bHiqual = false);
	void Reset();

	static bool Store(LPCTSTR szFileName, GLubyte *data, GLsizei width, GLsizei height);
};
