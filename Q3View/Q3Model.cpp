#include "pch.h"
#include <list>
#include <sstream>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Q3Model.h"
#include "Resource.h"

namespace Detail
{
	// see also https://icculus.org/~phaethon/q3a/formats/md3format.html
	// models: https://ioquake3.org/extras/models/

	struct MD3HEADER
	{
		char id[4];
		int  nVersion;
		char szFileName[64];
		int  nFlags;
		int  nFrames;
		int  nTags;
		int  nMeshes;
		int  nMaxTextures;
		int  nFrameOffset;
		int  nTagOffset;
		int  nMeshOffset;
		int  nFileSize;
	};

	struct MD3FRAME
	{
		float fMins[3];
		float fMaxs[3];
		float fPos[3];
		float fScale;
		char  szName[16];
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
		char szName[64];
		int  nFlags;
		int  nFrames;
		int  nTextures;
		int  nVertices;
		int  nTriangles;
		int  nTriangleOffset;
		int  nTexSpecOffset;
		int  nTexCoordOffset;
		int  nVertexOffset;
		int  nMeshSize;
	};

	struct MD3ANIM
	{
		int nStartFrame;
		int nFrameCount;
		int nLoops;
		int nFramesPerSec;
	};
}

///////////////////////////////////////////////////////////////////////////////
// MD3MeshGeometry
///////////////////////////////////////////////////////////////////////////////

MD3MeshGeometry::MD3MeshGeometry(void* meta, unsigned char* data)
{
	Detail::MD3MESHHEADER *pHeader = reinterpret_cast<Detail::MD3MESHHEADER*>(meta);
	m_nFrames = pHeader->nFrames;
	m_nVertices = pHeader->nVertices;
	m_nTriangles = pHeader->nTriangles;

	glGenVertexArrays(1, &m_nVertexArray);
	ATLASSERT(m_nVertexArray != 0);

	glBindVertexArray(m_nVertexArray);

	glGenBuffers(1, &m_nVertexBuffer);
	ATLASSERT(m_nVertexBuffer != 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, (size_t)m_nVertices * m_nFrames * 3 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);

	GLshort* pRaw = reinterpret_cast<GLshort*>(data + pHeader->nVertexOffset);
	float *pVertices = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	for (int frame = 0; frame < m_nFrames; frame++)
		for (int vert = 0; vert < m_nVertices; vert++)
		{
			int base_in = (frame * m_nVertices * 4) + (vert * 4);
			int base_out = (frame * m_nVertices * 3) + (vert * 3);
			pVertices[base_out + 0] = pRaw[base_in + 0] / 64.0f;
			pVertices[base_out + 1] = pRaw[base_in + 1] / 64.0f;
			pVertices[base_out + 2] = pRaw[base_in + 2] / 64.0f;
			// pRaw[base_in + 3] contains normal encoded in spherical coordinates
		}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	if (pHeader->nTextures)
	{
		glGenBuffers(1, &m_nTexCoordBuffer);
		ATLASSERT(m_nTexCoordBuffer != 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_nTexCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER,
			(size_t)m_nVertices * 2 * sizeof(GLfloat),
			data + pHeader->nTexCoordOffset,
			GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	
	glGenBuffers(1, &m_nIndexBuffer);
	ATLASSERT(m_nIndexBuffer != 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		(size_t)m_nTriangles * 3 * sizeof(GLuint),
		data + pHeader->nTriangleOffset,
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

	if (m_nTexCoordBuffer)
	{
		glDeleteBuffers(1, &m_nTexCoordBuffer);
		m_nTexCoordBuffer = 0;
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
	ATLASSERT(m_nCurrentFrame < m_nFrames);
	size_t nVertexOffset = (size_t)m_nCurrentFrame * m_nVertices * 3 * sizeof(GLfloat);

	glBindVertexArray(m_nVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(nVertexOffset));
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
		ATLENSURE(pHeader->nFrameOffset == sizeof(Detail::MD3HEADER));
		ATLENSURE(pHeader->nFileSize == fi.uncompressed_size);

		size_t offset = sizeof(Detail::MD3HEADER);
		for (int i = 0; i < pHeader->nFrames; i++)
			offset += sizeof(Detail::MD3FRAME);

		ATLENSURE(offset == pHeader->nTagOffset);

		m_nTags = pHeader->nTags;
		m_nFrames = pHeader->nFrames;
		m_Tags.resize(pHeader->nFrames * (size_t)pHeader->nTags);

		for (int i = 0; i < pHeader->nTags; i++)
			for (int j = 0; j < pHeader->nFrames; j++)
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

				m_Tags[((size_t)i * pHeader->nFrames) + j] = MD3Tag(pTag->szTagName, matrix);

				offset += sizeof(Detail::MD3TAG);
			}

		ATLENSURE(offset == pHeader->nMeshOffset);

		for (int i = 0; i < pHeader->nMeshes; i++)
		{
			Detail::MD3MESHHEADER* pMeshHeader = reinterpret_cast<Detail::MD3MESHHEADER*>(&mem[offset]);
			ATLENSURE(strncmp(pMeshHeader->id, "IDP3", 4) == 0);
			AddChild(new MD3MeshGeometry(reinterpret_cast<void*>(pMeshHeader), &mem[offset]));
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

void MD3Mesh::Render(float time)
{
	m_fAnimationTime += time / 1000.0f; // ms to sec
	while (m_fAnimationTime >= 60.0f)
		m_fAnimationTime -= 60.0f;

	m_nCurrentFrame = static_cast<int>(m_fAnimationTime * m_Animation.m_nFramesPerSecond);
	m_nCurrentFrame = m_Animation.m_nLength == 0 ?
		m_Animation.m_nStart : (m_nCurrentFrame % m_Animation.m_nLength) + m_Animation.m_nStart;

	ForEachChild([this](CSceneGraphNode * pNode) {
		reinterpret_cast<MD3MeshGeometry*>(pNode)->SetCurrentFrame(m_nCurrentFrame);
	});

	CSceneGraphNode::Render(time);
}

glm::mat4 MD3Mesh::Transform(const std::string& name)
{
	if (m_nTags == 1) // shortcut
		return m_Tags[m_nCurrentFrame].transform;

	for (int i = 0; i < m_nTags; i++)
	{
		int index = m_nTags * m_nCurrentFrame + i;
		if (m_Tags[index].name.compare(name) == 0)
			return m_Tags[index].transform;
	}

	return glm::mat4(1.0f);
}

void MD3Mesh::Animate(const MD3AnimationInfo& info)
{
	m_Animation = info;
	m_fAnimationTime = 0.0f;
	m_nCurrentFrame = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Q3Model
///////////////////////////////////////////////////////////////////////////////

Q3Model::Q3Model(LPCTSTR szFile)
	: m_Animations(Q3_NUM_ANIMATIONS), 
	  m_matModelView(1.0f),
	  m_matProjection(1.0f)
{
	USES_CONVERSION;
	ATLTRACE(_T("Try loading model from \"%s\"...\n"), szFile);

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
				// ATLTRACE(_T("Zip entry: %s\n"), (LPCTSTR)CA2CT(szEntry));
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
			strcat_s(szEntry, "head_default.skin");
			ParseSkinFile("head", szEntry, zip);

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "upper_default.skin");
			ParseSkinFile("upper", szEntry, zip);

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "lower_default.skin");
			ParseSkinFile("lower", szEntry, zip);

			LoadTextures(zip);

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "head.md3");
			m_pHead.reset(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "upper.md3");
			m_pUpper.reset(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "lower.md3");
			m_pLower.reset(new MD3Mesh(szEntry, zip));

			strcpy_s(szEntry, szModelPath);
			strcat_s(szEntry, "animation.cfg");
			if (ParseAnimationScript(szEntry, zip))
			{
				m_pUpper->Animate(m_Animations[TORSO_STAND2]);
				m_pLower->Animate(m_Animations[LEGS_IDLE]);
			}
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
	ATLTRACE(_T("Model cleanup.\n"));

	for (auto iter = m_mapTextures.begin(); iter != m_mapTextures.end(); iter++)
		delete iter->second;
}

bool Q3Model::ParseAnimationScript(const std::string& name, unzFile source)
{
	USES_CONVERSION;
	unz_file_info fi;

	if (unzLocateFile(source, name.c_str(), 2) != UNZ_OK)
	{
		ATLTRACE(_T("File not found: \"%s\".\n"), (LPCTSTR)CA2CT(name.c_str()));
		return false;
	}
	
	ATLTRACE(_T("Loading \"%s\"...\n"), (LPCTSTR)CA2CT(name.c_str()));
	ATLENSURE(unzGetCurrentFileInfo(source, &fi, nullptr, 0, nullptr, 0, nullptr, 0) == UNZ_OK);

	std::vector<char> mem(fi.uncompressed_size);
	ATLENSURE(unzOpenCurrentFile(source) == UNZ_OK);
	ATLENSURE(unzReadCurrentFile(source, mem.data(), fi.uncompressed_size) == fi.uncompressed_size);
	ATLENSURE(unzCloseCurrentFile(source) == UNZ_OK);

	std::string line;
	std::string cfg(mem.begin(), mem.end());
	std::istringstream stream(cfg);
	std::list<Detail::MD3ANIM> animations;

	while (std::getline(stream, line))
	{
		if (line.length() > 0 && line[0] >= '0' && line[0] <= '9')
		{
			Detail::MD3ANIM anim;
			std::istringstream parser(line);
			parser >> anim.nStartFrame >> anim.nFrameCount >> anim.nLoops >> anim.nFramesPerSec;
			animations.push_back(anim);
		}
	}

	ATLTRACE(_T("Loaded %d animations."), animations.size());
	ATLENSURE(animations.size() == Q3_NUM_ANIMATIONS);

	int counter = 0;
	for (auto anim = animations.cbegin(); anim != animations.cend(); anim++)
	{
		m_Animations[counter].m_nStart = anim->nStartFrame;
		m_Animations[counter].m_nLength = anim->nFrameCount;
		m_Animations[counter].m_nLooping = anim->nLoops;
		m_Animations[counter].m_nFramesPerSecond = anim->nFramesPerSec;
		counter++;
	}
	
	int skip_torso = m_Animations[LEGS_WALKCR].m_nStart - m_Animations[TORSO_GESTURE].m_nStart;
	for (int i = LEGS_WALKCR; i < Q3_NUM_ANIMATIONS; i++)
		m_Animations[i].m_nStart -= skip_torso;

	return true;
}

bool Q3Model::ParseSkinFile(const std::string& id, const std::string& name, unzFile source)
{
	USES_CONVERSION;
	unz_file_info fi;

	if (unzLocateFile(source, name.c_str(), 2) != UNZ_OK)
	{
		ATLTRACE(_T("File not found: \"%s\".\n"), (LPCTSTR)CA2CT(name.c_str()));
		return false;
	}

	ATLTRACE(_T("Loading \"%s\"...\n"), (LPCTSTR)CA2CT(name.c_str()));
	ATLENSURE(unzGetCurrentFileInfo(source, &fi, nullptr, 0, nullptr, 0, nullptr, 0) == UNZ_OK);

	std::vector<char> mem(fi.uncompressed_size);
	ATLENSURE(unzOpenCurrentFile(source) == UNZ_OK);
	ATLENSURE(unzReadCurrentFile(source, mem.data(), fi.uncompressed_size) == fi.uncompressed_size);
	ATLENSURE(unzCloseCurrentFile(source) == UNZ_OK);

	std::string line;
	std::string cfg(mem.begin(), mem.end());
	std::istringstream stream(cfg);

	while (std::getline(stream, line))
	{
		line.erase(std::find_if(line.rbegin(), line.rend(), [](int ch) { 
			return !std::isspace(ch);
		}).base(), line.end());
		
		size_t sep = line.find(',');
		if (sep != std::string::npos && sep < line.length() - 1)
		{
			m_mapTextureIds[id] = line.substr(sep + 1);
			return true;
		}
	}

	return false;
}

void Q3Model::LoadTextures(unzFile source)
{
	USES_CONVERSION;
	unz_file_info fi;

	for (auto iter = m_mapTextureIds.begin(); iter != m_mapTextureIds.end(); iter++)
	{
		if (m_mapTextures.find(iter->second) == m_mapTextures.end())
		{
			CTexture* pTexture = new CTexture();
			
			if (unzLocateFile(source, iter->second.c_str(), 2) == UNZ_OK)
			{
				ATLTRACE(_T("Loading texture \"%s\"...\n"), (LPCTSTR)CA2CT(iter->second.c_str()));
				ATLENSURE(unzGetCurrentFileInfo(source, &fi, nullptr, 0, nullptr, 0, nullptr, 0) == UNZ_OK);

				std::vector<unsigned char> mem(fi.uncompressed_size);
				ATLENSURE(unzOpenCurrentFile(source) == UNZ_OK);
				ATLENSURE(unzReadCurrentFile(source, mem.data(), fi.uncompressed_size) == fi.uncompressed_size);
				ATLENSURE(unzCloseCurrentFile(source) == UNZ_OK);

				pTexture->Load(mem.data(), fi.uncompressed_size, true);
			}
			else
			{
				ATLTRACE(_T("File not found: \"%s\".\n"), (LPCTSTR)CA2CT(iter->second.c_str()));
			}

			m_mapTextures[iter->second] = pTexture;
		}
	}
}

GLuint Q3Model::GetTexture(const std::string& id) const
{
	auto name = m_mapTextureIds.find(id);
	if (name != m_mapTextureIds.end())
	{
		auto texture = m_mapTextures.find(name->second);
		if (texture != m_mapTextures.end())
		{
			return *(texture->second);
		}
	}
	return 0; // not found
}

void Q3Model::SetAnimation(int id)
{
	ATLASSERT(id >= 0 && id < Q3_NUM_ANIMATIONS);

	if (id >= BOTH_DEATH1 && id <= BOTH_DEAD3)
	{
		m_pUpper->Animate(m_Animations[id]);
		m_pLower->Animate(m_Animations[id]);
	}

	if (id >= TORSO_GESTURE && id <= TORSO_STAND2)
	{
		m_pUpper->Animate(m_Animations[id]);
	}

	if (id >= LEGS_WALKCR && id <= LEGS_TURN)
	{
		m_pLower->Animate(m_Animations[id]);
	}
}

void Q3Model::Render(float time)
{
	glUseProgram(m_Shader);

	if (m_bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

	glUniform1i(
		glGetUniformLocation(m_Shader, "wireframe"),
		m_bWireframe ? 1 : 0
	);

	m_matModelView = glm::rotate(
		m_matModelView,
		glm::radians(-time / 50.0f),
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
		glBindTexture(GL_TEXTURE_2D, GetTexture("lower"));
		m_pLower->Render(time);

		transform = transform * m_pLower->Transform("tag_torso");
		glUniformMatrix4fv(
			glGetUniformLocation(m_Shader, "modelview"),
			1, GL_FALSE, glm::value_ptr(transform)
		);
	}

	if (m_pUpper)
	{
		glBindTexture(GL_TEXTURE_2D, GetTexture("upper"));
		m_pUpper->Render(time);

		transform = transform * m_pUpper->Transform("tag_head");
		glUniformMatrix4fv(
			glGetUniformLocation(m_Shader, "modelview"),
			1, GL_FALSE, glm::value_ptr(transform)
		);
	}

	if (m_pHead)
	{
		glBindTexture(GL_TEXTURE_2D, GetTexture("head"));
		m_pHead->Render(time);
	}

	if (m_bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindTexture(GL_TEXTURE_2D, 0);
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
