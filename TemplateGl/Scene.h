#pragma once
class CScene
{
public:
	CScene();
	~CScene();

	void Render(double time);
	void Resize(int width, int height);
};

