#pragma once

class CShaderProgram
{
private:
	GLuint m_nShaderProgram = 0;
	CString m_strLastError;

protected:
	static bool LoadShaderSource(
		HINSTANCE hInst,
		LPCTSTR szResType,
		int nResId,
		CStringA& shader);

public:
	CShaderProgram();
	~CShaderProgram();

	bool Create(
		HINSTANCE hInst,
		LPCTSTR szResType,
		int nResVertexShader,
		int nResFragmentShader);

	void Destroy();

	LPCTSTR GetLastError() const { return m_strLastError; }
	operator GLuint() const { return m_nShaderProgram; }
};
