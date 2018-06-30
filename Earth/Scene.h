#pragma once

#include <Texture.h>
#include <SceneGraph.h>
#include <ShaderProgram.h>

class CScene : public CSceneGraphNode
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLsizei m_numVertices = 0;

	glm::mat4 m_matTransformation;
	CTexture m_texEarth;
	CShaderProgram m_ShaderProgram;

public:
	CScene();
	virtual ~CScene();

	virtual bool Create();
	virtual void Resize(int width, int height);
	virtual void Render(float time);
};
