#pragma once

#include <Texture.h>
#include <SceneGraph.h>
#include <ShaderProgram.h>

constexpr int EVT_MVP_UPDATE = 1;
constexpr int EVT_RESOURCE_READY = 2;

class CForeground : public CSceneGraphNode
{
private:
	glm::mat4 m_matTransformation;

public:
	CForeground();
	virtual ~CForeground();

	virtual void Resize(int width, int height);
	virtual void Render(float time);
};

class CEarthBall : public CSceneGraphNode
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLsizei m_numVertices = 0;

	glm::mat4 m_matTransformation;
	CTexture m_texEarth;
	CShaderProgram m_ShaderProgram;

public:
	CEarthBall();
	virtual ~CEarthBall();

	virtual bool Create();
	virtual void Handle(int evt, int flags, void* param);
	virtual void Render(float time);
};

class CParticles : public CSceneGraphNode
{
private:
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLsizei m_numVertices = 0;

	glm::mat4 m_matTransformation;
	CShaderProgram m_ShaderProgram;
public:
	CParticles();
	virtual ~CParticles();

	virtual bool Create();
	virtual void Handle(int evt, int flags, void* param);
	virtual void Render(float time);
};
