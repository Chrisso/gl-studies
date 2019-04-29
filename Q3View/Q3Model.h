#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <ShaderProgram.h>
#include <SceneGraph.h>

class MD3Tag
{
public:
	std::string name;
	glm::mat4 transform;

	MD3Tag() : transform(1.0f)
	{
	}

	MD3Tag(const std::string& id, glm::mat4 mat) : name(id), transform(mat)
	{
	}

	MD3Tag& operator=(const MD3Tag& other) = default;
};

class MD3AnimationInfo
{
public:
	int m_nStart = 0;
	int m_nLength = 0;
	int m_nLooping = 0;
	int m_nFramesPerSecond = 1;

public:
	MD3AnimationInfo() = default;
};

class MD3MeshGeometry : public CSceneGraphNode
{
private:
	int m_nFrames;
	int m_nVertices;
	int m_nTriangles;
	int m_nCurrentFrame = 0;

	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLuint m_nIndexBuffer = 0;

public:
	MD3MeshGeometry(int frames, int verts, int triangles, int triangle_offset, int vertex_offset, unsigned char* data);
	virtual ~MD3MeshGeometry();
	virtual void Render(float time);
	
	int GetCurrentFrame() const { return m_nCurrentFrame; }
	void SetCurrentFrame(int frame) { m_nCurrentFrame = frame; }
};

class MD3Mesh : public CSceneGraphNode
{
private:
	std::string m_sName;
	std::vector<MD3Tag> m_Tags;
	int m_nTags = 0;
	int m_nBoneFrames = 0;

	MD3AnimationInfo m_Animation;
	float m_fAnimationTime = 0.0f;
	int   m_nCurrentFrame = 0;

public:
	MD3Mesh(const std::string& name, unzFile source);
	virtual ~MD3Mesh();
	virtual void Render(float time);

	glm::mat4 Transform(const std::string& name);
	void Animate(const MD3AnimationInfo& info);
};

class Q3Model : public CSceneGraphNode
{
private:
	CShaderProgram m_Shader;
	std::unique_ptr<MD3Mesh> m_pLower;
	std::unique_ptr<MD3Mesh> m_pUpper;
	std::unique_ptr<MD3Mesh> m_pHead;
	std::vector<MD3AnimationInfo> m_Animations;
	glm::mat4 m_matProjection;
	glm::mat4 m_matModelView;

	bool ParseAnimationScript(const std::string& name, unzFile source);

public:
	Q3Model(LPCTSTR szFile);
	virtual ~Q3Model();
	virtual void Render(float time);
	virtual void Resize(int width, int height);
};

#define Q3_NUM_ANIMATIONS 25

#define BOTH_DEATH1    0
#define BOTH_DEAD1     1
#define BOTH_DEATH2    2
#define BOTH_DEAD2     3
#define BOTH_DEATH3    4
#define BOTH_DEAD3     5

#define TORSO_GESTURE  6
#define TORSO_ATTACK   7
#define TORSO_ATTACK2  8
#define TORSO_DROP     9
#define TORSO_RAISE   10
#define TORSO_STAND   11
#define TORSO_STAND2  12

#define LEGS_WALKCR   13
#define LEGS_WALK     14
#define LEGS_RUN      15
#define LEGS_BACK     16
#define LEGS_SWIM     17
#define LEGS_JUMP     18
#define LEGS_LAND     19
#define LEGS_JUMPB    20
#define LEGS_LANDB    21
#define LEGS_IDLE     22
#define LEGS_IDLECR   23
#define LEGS_TURN     24
