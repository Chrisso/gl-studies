#pragma once

class CScene
{
private:
	GLuint m_nVertexBuffer = 0;

public:
	CScene();
	~CScene();

	void Render(float time);
	void Resize(int width, int height);
};
