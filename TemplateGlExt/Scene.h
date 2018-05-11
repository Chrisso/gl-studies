#pragma once

#include <glm/glm.hpp>

class CScene
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLuint m_nShaderProgram = 0;

	glm::mat4 m_matMVP;

protected:
	static bool LoadShaderSource(int id, CStringA& shader);

public:
	CScene();
	~CScene();

	bool Create();
	void Render(float time);
	void Resize(int width, int height);
};
