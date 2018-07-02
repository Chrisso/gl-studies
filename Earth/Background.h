#pragma once

#include <Texture.h>
#include <SceneGraph.h>
#include <ShaderProgram.h>

// fullscreen textured quad (triangle strip)
class CBackground : public CSceneGraphNode
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;

	CTexture m_Texture;
	CShaderProgram m_ShaderProgram;

public:
	CBackground();
	virtual ~CBackground();

	virtual bool Create();
	virtual void Resize(int width, int height);
	virtual void Render(float time);
};
