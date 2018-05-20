#pragma once

#include <glm/glm.hpp>
#include <ShaderProgram.h>
#include <TextRendering.h>

class CScene
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	glm::mat4 m_matSceneMVP;
	glm::mat4 m_matHudMVP;

	CShaderProgram *m_pShaderProgram = nullptr;
	CShaderProgram *m_pFontShader = nullptr;

	CTextureFont *m_pTextureFont = nullptr;
	CRenderString *m_pRenderString = nullptr;

public:
	CScene();
	~CScene();

	bool Create();
	void Render(float time);
	void Resize(int width, int height);
};
