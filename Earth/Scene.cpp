#include "stdafx.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "resource.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CScene::CScene()
{

}

CScene::~CScene()
{
	ATLTRACE(_T("Cleaning up...\n"));

	if (m_nVertexArray)
	{
		glDeleteVertexArrays(1, &m_nVertexArray);
		m_nVertexArray = 0;
	}

	if (m_nVertexBuffer)
	{
		glDeleteBuffers(1, &m_nVertexBuffer);
		m_nVertexBuffer = 0;
	}
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

namespace detail
{
	void spheric_subdivision(
		glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
		std::vector<glm::vec3>& vertices,
		int recurse)
	{
		if (recurse == 0)
		{
			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
			return;
		}

		glm::vec3 m1 = glm::normalize(v1 + v2);
		glm::vec3 m2 = glm::normalize(v1 + v3);
		glm::vec3 m3 = glm::normalize(v2 + v3);
		spheric_subdivision(v1, m1, m2, vertices, recurse - 1);
		spheric_subdivision(v3, m2, m3, vertices, recurse - 1);
		spheric_subdivision(v2, m3, m1, vertices, recurse - 1);
		spheric_subdivision(m1, m3, m2, vertices, recurse - 1);
	}

	void triangulate_sphere(std::vector<glm::vec3>& vertices, int depth)
	{
		// https://en.wikipedia.org/wiki/Tetrahedron
		glm::vec3 v1(sqrtf(8.0f / 9.0f), 0.0f, -1.0f / 3.0f);
		glm::vec3 v2(-sqrtf(2.0f / 9.0f), sqrtf(2.0f / 3.0f), -1.0f / 3.0f);
		glm::vec3 v3(-sqrtf(2.0f / 9.0f), -sqrtf(2.0f / 3.0f), -1.0f / 3.0f);
		glm::vec3 v4(0.0f, 0.0f, 1.0f);

		spheric_subdivision(v1, v3, v2, vertices, depth);
		spheric_subdivision(v1, v4, v3, vertices, depth);
		spheric_subdivision(v1, v2, v4, vertices, depth);
		spheric_subdivision(v2, v3, v4, vertices, depth);
	}
}

#ifdef _DEBUG
	#define SPHERE_TRIANGULATION_DEPTH 4
#else
	#define SPHERE_TRIANGULATION_DEPTH 5
#endif

bool CScene::Create()
{
	if (!m_ShaderProgram.CreateSimple(
		_Module.GetResourceInstance(),
		_T("GLSL_SHADER"),
		IDR_GLSL_VERTEX_SHADER,
		IDR_GLSL_FRAGMENT_SHADER))
		return false;

	glGenVertexArrays(1, &m_nVertexArray);
	if (!m_nVertexArray) return false;
	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexBuffer);
	if (!m_nVertexBuffer) return false;
	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);

	// Well, we could stream directly into mapped GPU memory but
	// the algorithmic complexity seems not worth it. Thus stream
	// to a vector buffer and copy the whole thing when completed.

	std::vector<glm::vec3> vertices;
	vertices.reserve(12 * (size_t)std::pow(4, SPHERE_TRIANGULATION_DEPTH));
	detail::triangulate_sphere(vertices, SPHERE_TRIANGULATION_DEPTH);

	m_numVertices = (GLsizei)vertices.size();
	ATLTRACE(_T("#Sphere-Vertices: %d\n"), m_numVertices);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * m_numVertices, NULL, GL_STATIC_DRAW);
	float *pGeometry = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	size_t offset = 0;
	for (glm::vec3& v : vertices)
	{
		pGeometry[offset++] = v[0];
		pGeometry[offset++] = v[1];
		pGeometry[offset++] = v[2];
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

void CScene::Resize(int width, int height)
{
	if (height == 0) height = 1; // avoid division by zero

	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		(float)width / (float)height,
		1.0f, 8.0f);

	glm::mat4 modelview = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	m_matTransformation = projection * modelview;
}

void CScene::Render(float time)
{
	glUseProgram(m_ShaderProgram);

	m_matTransformation = glm::rotate(
		m_matTransformation,
		glm::radians(-time / 50.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glUniformMatrix4fv(
		glGetUniformLocation(m_ShaderProgram, "transformation"),
		1, GL_FALSE, glm::value_ptr(m_matTransformation)
	);

	if (m_nVertexArray && m_nVertexBuffer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
		glBindVertexArray(m_nVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(0);
}
