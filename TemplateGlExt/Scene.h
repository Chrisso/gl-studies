#pragma once

#include <glm/glm.hpp>
#include <ShaderProgram.h>

class CScene
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	glm::mat4 m_matMVP;

	CShaderProgram *m_pShaderProgram = nullptr;

public:
	CScene();
	~CScene();

	bool Create();
	void Render(float time);
	void Resize(int width, int height);
};
