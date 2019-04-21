#pragma once

#include <string>
#include <SceneGraph.h>

class MD3Mesh : public CSceneGraphNode
{
private:
	std::string m_sName;

public:
	MD3Mesh(std::string name, unzFile source);
	virtual ~MD3Mesh();
};

class Q3Model : public CSceneGraphNode
{
public:
	Q3Model(LPCTSTR szFile);
	virtual ~Q3Model();
};
