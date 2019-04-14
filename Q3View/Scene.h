#pragma once

#include <glm/glm.hpp>
#include <SceneGraph.h>
#include <ShaderProgram.h>

class CScene : public CSceneGraphNode
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	glm::mat4 m_matMVP;

	CShaderProgram m_ShaderProgram;

public:
	CScene();
	virtual ~CScene();

	virtual bool Create();
	virtual void Render(float time);
	virtual void Resize(int width, int height);
};
