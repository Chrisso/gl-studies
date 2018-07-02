#pragma once

#include <list>

class CSceneGraphNode
{
protected:
	std::list<CSceneGraphNode*> m_pChildren;

public:
	CSceneGraphNode() = default;

	virtual ~CSceneGraphNode()
	{
		for (CSceneGraphNode* child : m_pChildren)
			delete child;
	}

	virtual bool Create()
	{
		return true;
	}

	virtual void Resize(int width, int height)
	{
		for (CSceneGraphNode* child : m_pChildren)
			child->Resize(width, height);
	}

	virtual void Render(float time)
	{
		for (CSceneGraphNode* child : m_pChildren)
			child->Render(time);
	}

	virtual void AddChild(CSceneGraphNode *child)
	{
		m_pChildren.push_back(child);
	}
};
