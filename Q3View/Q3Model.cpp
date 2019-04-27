#include "pch.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Q3Model.h"
#include "Resource.h"

namespace Detail
{
	// see also https://icculus.org/~phaethon/q3a/formats/md3format.html

	struct MD3HEADER
	{
		char id[4];
		int  nVersion;
		char szFileName[68];
		int  nBoneFrames;
		int  nTags;
		int  nMeshes;
		int  nMaxTextures;
		int  nHeaderSize;
		int  nTagOffset;
		int  nMeshOffset;
		int  nFileSize;
	};

	struct MD3BONEFRAME
	{
		float fMins[3];
		float fMaxs[3];
		float fPos[3];
		float fScale;
		char  cReserved[16];
	};

	struct MD3TAG
	{
		char  szTagName[64];
		float fPos[3];
		float fMatrix[3][3];
	};

	struct MD3MESHHEADER
	{
		char id[4];
		char szName[68];
		int  nFrames;
		int  nTextures;
		int  nVertices;
		int  nTriangles;
		int  nTriangleOffset;
		int  nHeaderSize;
		int  nTexCoordOffset;
		int  nVertexOffset;
		int  nMeshSize;
	};
}

///////////////////////////////////////////////////////////////////////////////
// MD3MeshGeometry
///////////////////////////////////////////////////////////////////////////////

MD3MeshGeometry::MD3MeshGeometry(
	int frames, int verts, int triangles,
	int triangle_offset, int vertex_offset,
	unsigned char* data)
	: m_nFrames(frames), m_nVertices(verts), m_nTriangles(triangles)
{
	glGenVertexArrays(1, &m_nVertexArray);
	ATLASSERT(m_nVertexArray != 0);

	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexBuffer);
	ATLASSERT(m_nVertexBuffer != 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, (size_t)verts * frames * 3 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);

	GLshort* pRaw = reinterpret_cast<GLshort*>(data + vertex_offset);
	float *pVertices = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	for (int frame = 0; frame < frames; frame++)
		for (int vert = 0; vert < verts; vert++)
		{
			int base_in = (frame * verts * 4) + (vert * 4);
			int base_out = (frame * verts * 3) + (vert * 3);
			pVertices[base_out + 0] = pRaw[base_in + 0] / 64.0f;
			pVertices[base_out + 1] = pRaw[base_in + 1] / 64.0f;
			pVertices[base_out + 2] = pRaw[base_in + 2] / 64.0f;
			// pRaw[base_in + 3] contains normal encoded in spherical coordinates
		}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	glGenBuffers(1, &m_nIndexBuffer);
	ATLASSERT(m_nIndexBuffer != 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		(size_t)triangles * 3 * sizeof(GLuint),
		data + triangle_offset,
		GL_STATIC_DRAW);

	glBindVertexArray(0);
}

MD3MeshGeometry::~MD3MeshGeometry()
{
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

	if (m_nIndexBuffer)
	{
		glDeleteBuffers(1, &m_nIndexBuffer);
		m_nIndexBuffer = 0;
	}
}

void MD3MeshGeometry::Render(float time)
{
	glBindVertexArray(m_nVertexArray);
	glDrawElements(GL_TRIANGLES, m_nTriangles * 3, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////
// MD3Mesh
///////////////////////////////////////////////////////////////////////////////

MD3Mesh::MD3Mesh(const std::string& name, unzFile source) : m_sName(name)
{
	USES_CONVERSION;
	unz_file_info fi;

	if (unzLocateFile(source, m_sName.c_str(), 2) == UNZ_OK)
	{
		ATLTRACE(_T("Loading \"%s\"...\n"), (LPCTSTR)CA2CT(m_sName.c_str()));
		ATLENSURE(unzGetCurrentFileInfo(source, &fi, nullptr, 0, nullptr, 0, nullptr, 0) == UNZ_OK);

		std::vector<unsigned char> mem(fi.uncompressed_size);
		ATLENSURE(unzOpenCurrentFile(source) == UNZ_OK);
		ATLENSURE(unzReadCurrentFile(source, mem.data(), fi.uncompressed_size) == fi.uncompressed_size);
		ATLENSURE(unzCloseCurrentFile(source) == UNZ_OK);

		Detail::MD3HEADER* pHeader = reinterpret_cast<Detail::MD3HEADER*>(mem.data());
		ATLENSURE(strncmp(pHeader->id, "IDP3", 4) == 0);
		ATLENSURE(pHeader->nHeaderSize == sizeof(Detail::MD3HEADER));
		ATLENSURE(pHeader->nFileSize == fi.uncompressed_size);

		size_t offset = sizeof(Detail::MD3HEADER);
		for (int i = 0; i < pHeader->nBoneFrames; i++)
			offset += sizeof(Detail::MD3BONEFRAME);

		ATLENSURE(offset == pHeader->nTagOffset);

		m_nTags = pHeader->nTags;
		m_nBoneFrames = pHeader->nBoneFrames;
		m_Tags.resize(pHeader->nBoneFrames * (size_t)pHeader->nTags);

		for (size_t i = 0; i < pHeader->nTags; i++)
			for (size_t j = 0; j < pHeader->nBoneFrames; j++)
			{
				Detail::MD3TAG* pTag = reinterpret_cast<Detail::MD3TAG*>(&mem[offset]);

				glm::mat4 matrix(1.0f);
				for (int x = 0; x < 3; x++)
					for (int y = 0; y < 3; y++)
						matrix[x][y] = pTag->fMatrix[x][y];

				// translation
				matrix[3][0] = pTag->fPos[0];
				matrix[3][1] = pTag->fPos[1];
				matrix[3][2] = pTag->fPos[2];

				m_Tags[(i * pHeader->nBoneFrames) + j] = MD3Tag(pTag->szTagName, matrix);

				offset += sizeof(Detail::MD3TAG);
			}

		ATLENSURE(offset == pHeader->nMeshOffset);

		for (size_t i = 0; i < pHeader->nMeshes; i++)
		{
			Detail::MD3MESHHEADER* pMeshHeader = reinterpret_cast<Detail::MD3MESHHEADER*>(&mem[offset]);
			ATLENSURE(strncmp(pMeshHeader->id, "IDP3", 4) == 0);
			AddChild(new MD3MeshGeometry(
				pMeshHeader->nFrames,
				pMeshHeader->nVertices,
				pMeshHeader->nTriangles,
				pMeshHeader->nTriangleOffset,
				pMeshHeader->nVertexOffset,
				&mem[offset]
			));
			offset += pMeshHeader->nMeshSize;
		}

		ATLENSURE(offset == pHeader->nFileSize);
	}
	else ATLTRACE(_T("Failed loading \"%s\"...\n"), (LPCTSTR)CA2CT(m_sName.c_str()));
}

MD3Mesh::~MD3Mesh()
{
	ATLTRACE(_T("Cleaning \"%s\".\n"), (LPCTSTR)CA2CT(m_sName.c_str()));
}

glm::mat4 MD3Mesh::Transform(const std::string& name)
{
	for (int i = 0; i < m_nTags; i++)
	{
		if (m_Tags[i].name.compare(name) == 0)
		{
			return m_Tags[i].transform;
		}
	}

	return glm::mat4(1.0f);
}

///////////////////////////////////////////////////////////////////////////////
// Q3Model
///////////////////////////////////////////////////////////////////////////////

Q3Model::Q3Model(LPCTSTR szFile)
{
	USES_CONVERSION;
	ATLTRACE(_T("Try loading model from %s...\n"), szFile);

	unzFile zip = unzOpen(CT2CA(szFile));
	if (zip)
	{
		int result = unzGoToFirstFile(zip);

		unz_file_info fi;
		char szEntry[MAX_PATH];
		char szModelPath[MAX_PATH];
		memset(szModelPath, 0, MAX_PATH);

		// find model path by iterating over all compressed files
		while (result == UNZ_OK)
		{
			if (unzGetCurrentFileInfo(zip, &fi, szEntry, MAX_PATH, nullptr, 0, nullptr, 0) == UNZ_OK)
			{
				ATLTRACE(_T("Zip entry: %s\n"), (LPCTSTR)CA2CT(szEntry));
				if (fi.size_filename > 8 && _stricmp(szEntry + fi.size_filename - 8, "head.md3") == 0)
				{
					strncpy_s(szModelPath, szEntry, fi.size_filename - 8);
					// break;
				}
			}

			result = unzGoToNextFile(zip);
		}

		if (strlen(szModelPath))
		{
			ATLTRACE(_T("Model path: %s\n"), (LPCTSTR)CA2CT(szModelPath));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "head.md3");
			m_pHead.reset(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "upper.md3");
			m_pUpper.reset(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "lower.md3");
			m_pLower.reset(new MD3Mesh(szEntry, zip));
		}

		unzClose(zip);
	}
	else ATLTRACE(_T("Load failed. Could not open file!\n"));

	ATLTRACE(_T("Initializing model shaders...\n"));

	if (!m_Shader.CreateSimple(
		_Module.GetResourceInstance(), _T("GLSL_SHADER"),
		IDR_GLSL_VERTEX_SHADER,
		IDR_GLSL_FRAGMENT_SHADER))
	{
		ATLTRACE(_T("Could not load shaders!\n"));
	}
}

Q3Model::~Q3Model()
{
}

void Q3Model::Render(float time)
{
	glUseProgram(m_Shader);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

	m_matModelView = glm::rotate(
		m_matModelView,
		glm::radians(-time / 20.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glm::mat4 transform = glm::rotate(
		m_matModelView,
		glm::radians(-90.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	glUniformMatrix4fv(
		glGetUniformLocation(m_Shader, "projection"),
		1, GL_FALSE, glm::value_ptr(m_matProjection)
	);

	glUniformMatrix4fv(
		glGetUniformLocation(m_Shader, "modelview"),
		1, GL_FALSE, glm::value_ptr(transform)
	);

	if (m_pLower)
	{
		m_pLower->Render(time);
		transform = transform * m_pLower->Transform("tag_torso");
		glUniformMatrix4fv(
			glGetUniformLocation(m_Shader, "modelview"),
			1, GL_FALSE, glm::value_ptr(transform)
		);
	}

	if (m_pUpper)
	{
		m_pUpper->Render(time);
		transform = transform * m_pUpper->Transform("tag_head");
		glUniformMatrix4fv(
			glGetUniformLocation(m_Shader, "modelview"),
			1, GL_FALSE, glm::value_ptr(transform)
		);
	}

	if (m_pHead)
	{
		m_pHead->Render(time);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}

void Q3Model::Resize(int width, int height)
{
	if (height == 0) height = 1; // avoid division by zero

	m_matProjection = glm::perspective(
		glm::radians(60.0f),
		(float)width / (float)height,
		1.0f, 100.0f);

	m_matModelView = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 75.0f),
		glm::vec3(0.0f, 5.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}
