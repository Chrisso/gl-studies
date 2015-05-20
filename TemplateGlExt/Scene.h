#pragma once
class CScene
{
private:
	unsigned int m_nVertexBuffer;

public:
	CScene();
	~CScene();

	void Render(double time);
	void Resize(int width, int height);
};

