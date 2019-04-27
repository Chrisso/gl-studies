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

class MD3MeshGeometry : public CSceneGraphNode
{
private:
	int m_nFrames;
	int m_nVertices;
	int m_nTriangles;

	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLuint m_nIndexBuffer = 0;

public:
	MD3MeshGeometry(int frames, int verts, int triangles, int triangle_offset, int vertex_offset, unsigned char* data);
	virtual ~MD3MeshGeometry();
	virtual void Render(float time);
};

class MD3Mesh : public CSceneGraphNode
{
private:
	std::string m_sName;
	std::vector<MD3Tag> m_Tags;
	int m_nTags = 0;
	int m_nBoneFrames = 0;

public:
	MD3Mesh(const std::string& name, unzFile source);
	virtual ~MD3Mesh();
	glm::mat4 Transform(const std::string& name);
};

class Q3Model : public CSceneGraphNode
{
private:
	CShaderProgram m_Shader;
	std::unique_ptr<MD3Mesh> m_pLower;
	std::unique_ptr<MD3Mesh> m_pUpper;
	std::unique_ptr<MD3Mesh> m_pHead;
	glm::mat4 m_matProjection;
	glm::mat4 m_matModelView;

public:
	Q3Model(LPCTSTR szFile);
	virtual ~Q3Model();
	virtual void Render(float time);
	virtual void Resize(int width, int height);
};
