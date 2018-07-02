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
		vertices.clear();
		vertices.reserve(60 * (size_t)std::pow(4, depth));

		// https://en.wikipedia.org/wiki/Icosahedron#Cartesian_coordinates
		glm::vec3 v = glm::normalize(glm::vec3(1.0f, 0.0f, (1.0f + std::sqrtf(5)) / 2.0f));

		// https://www.cs.cmu.edu/~fp/courses/graphics/code/08-shading_code/subdivide.c
		glm::vec3 vs[12] = {
			glm::vec3(-v.x, 0.0f, v.z),  glm::vec3(v.x, 0.0f, v.z),
			glm::vec3(-v.x, 0.0f, -v.z), glm::vec3(v.x, 0.0f, -v.z),
			glm::vec3(0.0f, v.z, v.x),   glm::vec3(0.0f, v.z, -v.x),
			glm::vec3(0.0f, -v.z, v.x),  glm::vec3(0.0f, -v.z, -v.x),
			glm::vec3(v.z, v.x, 0.0f),   glm::vec3(-v.z, v.x, 0.0f),
			glm::vec3(v.z, -v.x, 0.0f),  glm::vec3(-v.z, -v.x, 0.0f)
		};

		GLubyte indices[20][3] = {
			{ 1,4,0 },{ 4,9,0 },{ 4,5,9 },{ 8,5,4 },{ 1,8,4 },
			{ 1,10,8 },{ 10,3,8 },{ 8,3,5 },{ 3,2,5 },{ 3,7,2 },
			{ 3,10,7 },{ 10,6,7 },{ 6,11,7 },{ 6,0,11 },{ 6,1,0 },
			{ 10,1,6 },{ 11,0,9 },{ 2,11,9 },{ 5,2,9 },{ 11,2,7 }
		};

		for (int i = 0; i < 20; i++)
			spheric_subdivision(
				vs[indices[i][0]],
				vs[indices[i][1]],
				vs[indices[i][2]],
				vertices,
				depth);

		ATLASSERT(vertices.size() == 60 * (size_t)std::pow(4, depth));
	}

	glm::vec2 gen_sphere_texcoords(const glm::vec3& v)
	{
		glm::vec2 result;
		result.s = 0.5f + (float)(std::atan2(v[0], v[2]) / (2 * M_PI));
		result.t = 0.5f - (float)(std::asin(v[1]) / M_PI);
		return result;
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

	if (m_texEarth == 0 || !m_texEarth.Load(
		_Module.GetResourceInstance(), _T("IMAGE_JPEG"), IDR_IMAGE_BLUE_MARBLE, true))
		return false;

	ATLTRACE(_T("Default texture size: %dx%d\n"), m_texEarth.GetWidth(), m_texEarth.GetHeight());

	if (GLEW_ARB_texture_filter_anisotropic)
	{
		float fMaxAniso = 0.0;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &fMaxAniso);
		ATLTRACE(_T("Max anisotropy: %f\n"), fMaxAniso);
	}
	else
	{
		ATLTRACE(_T("Anisotropic filtering is not supported.\n"));
	}

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
	detail::triangulate_sphere(vertices, SPHERE_TRIANGULATION_DEPTH);

	m_numVertices = (GLsizei)vertices.size();
	ATLTRACE(_T("#Sphere-Vertices: %d\n"), m_numVertices);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * m_numVertices, NULL, GL_STATIC_DRAW);
	float *pGeometry = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	for (size_t i = 0; i < vertices.size(); i += 3) // for each triangle
	{
		glm::vec2 texCoords[3] = {
			detail::gen_sphere_texcoords(vertices[i + 0]),
			detail::gen_sphere_texcoords(vertices[i + 1]),
			detail::gen_sphere_texcoords(vertices[i + 2])
		};

		// check for 360 deg edge, https://gamedev.stackexchange.com/a/34070
		glm::vec3 v1 = glm::vec3(texCoords[0] - texCoords[1], 0.0f);
		glm::vec3 v2 = glm::vec3(texCoords[2] - texCoords[1], 0.0f);
		glm::vec3 vn = glm::cross(v1, v2);

		if (vn.z <= 0) // edge crossed
		{
			if (texCoords[0].s > 0.9f) texCoords[0].s -= 1.0f;
			if (texCoords[1].s > 0.9f) texCoords[1].s -= 1.0f;
			if (texCoords[2].s > 0.9f) texCoords[2].s -= 1.0f;
		}

		for (size_t offset = 0; offset < 3; offset++)
		{
			pGeometry[5 * (i + offset) + 0] = vertices[i + offset].x;
			pGeometry[5 * (i + offset) + 1] = vertices[i + offset].y;
			pGeometry[5 * (i + offset) + 2] = vertices[i + offset].z;
			pGeometry[5 * (i + offset) + 3] = texCoords[offset].s;
			pGeometry[5 * (i + offset) + 4] = texCoords[offset].t;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

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
		glm::vec3(0.0f, 1.0f, 3.0f),
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
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
		glBindTexture(GL_TEXTURE_2D, m_texEarth);
		glBindVertexArray(m_nVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(0);
}
