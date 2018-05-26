#pragma once

class CShader
{
private:
	GLuint m_nShader = 0;
	CString m_strInfoLog;

protected:
	static bool LoadSource(
		HINSTANCE hInst,
		LPCTSTR szResType,
		int nResId,
		CStringA& shader);

public:
	CShader();
	~CShader();

	bool Create(GLenum nType, HINSTANCE hInst, PCTSTR szResType, int nResId);

	LPCTSTR GetInfoLog() const { return m_strInfoLog; }
	operator GLuint() const { return m_nShader; }
};

class CShaderProgram
{
private:
	GLuint m_nShaderProgram = 0;
	CString m_strInfoLog;

public:
	CShaderProgram();
	~CShaderProgram();

	bool Create();
	void Attach(GLuint nShader);
	void Detach(GLuint nShader);
	bool Link();
	void Destroy();

	bool CreateSimple(
		HINSTANCE hInst,
		LPCTSTR szResType,
		int nResVertexShader,
		int nResFragmentShader);

	LPCTSTR GetInfoLog() const { return m_strInfoLog; }
	operator GLuint() const { return m_nShaderProgram; }
};
