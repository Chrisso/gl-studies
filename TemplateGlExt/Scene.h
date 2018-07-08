#pragma once

#include <glm/glm.hpp>
#include <SceneGraph.h>
#include <ShaderProgram.h>
#include <TextRendering.h>

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

class CHud : public CSceneGraphNode
{
private:
	glm::mat4 m_matMVP;

	CShaderProgram m_ShaderProgram;
	CTextureFont m_TextureFont;
	CRenderString m_RenderString;

public:
	CHud();
	virtual ~CHud();

	virtual bool Create();
	virtual void Render(float time);
	virtual void Resize(int width, int height);
};
